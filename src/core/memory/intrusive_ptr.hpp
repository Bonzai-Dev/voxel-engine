/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <cstdint>
#include <atomic>

namespace Core {
  // Base class for all reference-counted objects
  // The user is expected to manage the lifetime themselves by overriding addRef and release methods
  // Example usage:
  // class Texture: public RefCounted { ... }
  class RefCounted {
    public:
      virtual ~RefCounted() = default;

      virtual unsigned long addRef() {
        // Implementation example:
        //
        // return ++referenceCount;
        return 0;
      }

      virtual unsigned long release() {
        // Implementation example:
        //
        // unsigned long result = --referenceCount;
        // if (result == 0) {
        //   delete this;
        // }
        // return result;
        return 0;
      }

      // virtual unsigned long addRef(std::memory_order memoryOrder) {
      //   return ++referenceCount;
      // }
      //
      // virtual unsigned long release(std::memory_order memoryOrder) {
      //   unsigned long result = --referenceCount;
      //   if (result == 0) {
      //     delete this;
      //   }
      //   return result;
      // }

      unsigned long getRefCount() {
        return referenceCount.load();
      }

    protected:
      mutable std::atomic<unsigned long> referenceCount = 1;
  };

  // Class that contains the pointer
  template<typename T>
  class IntrusivePtr {
    public:
      typedef T InterfaceType;

      // template<bool b, typename U = void>
      // struct EnableIf {
      // };

      // template<typename U>
      // struct EnableIf<true, U> {
      //   typedef U type;
      // };

    protected:
      InterfaceType *instance;
      template<class U>
      friend class IntrusivePtr;

      void internalAddRef() const noexcept {
        if (instance != nullptr) {
          instance->addRef();
        }
      }

      unsigned long internalRelease() noexcept {
        unsigned long ref = 0;
        T *temp = instance;

        if (temp != nullptr) {
          instance = nullptr;
          ref = temp->release();
        }

        return ref;
      }

    public:
      IntrusivePtr() noexcept: instance(nullptr) {
      }

      IntrusivePtr(std::nullptr_t) noexcept: instance(nullptr) {
      }

      template<class U>
      IntrusivePtr(U *other) noexcept: instance(other) {
        internalAddRef();
      }

      IntrusivePtr(const IntrusivePtr &other) noexcept: instance(other.instance) {
        internalAddRef();
      }

      // copy ctor that allows to instantiate class when U* is convertible to T*
      template<class U>
      IntrusivePtr(const IntrusivePtr<U> &other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: instance(
        other.instance) {
        internalAddRef();
      }

      IntrusivePtr(IntrusivePtr &&other) noexcept: instance(nullptr) {
        if (this != reinterpret_cast<IntrusivePtr *>(&reinterpret_cast<unsigned char &>(other))) {
          swap(other);
        }
      }

      // Move ctor that allows instantiation of a class when U* is convertible to T*
      template<class U>
      IntrusivePtr(IntrusivePtr<U> &&other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: instance(
        other.instance) {
        other.instance = nullptr;
      }

      ~IntrusivePtr() noexcept {
        internalRelease();
      }

      IntrusivePtr &operator=(std::nullptr_t) noexcept {
        internalRelease();
        return *this;
      }

      IntrusivePtr &operator=(T *other) noexcept {
        if (instance != other) {
          IntrusivePtr(other).swap(*this);
        }
        return *this;
      }

      template<typename U>
      IntrusivePtr &operator=(U *other) noexcept {
        IntrusivePtr(other).swap(*this);
        return *this;
      }

      IntrusivePtr &operator=(const IntrusivePtr &other) noexcept // NOLINT(bugprone-unhandled-self-assignment)
      {
        if (instance != other.instance) {
          IntrusivePtr(other).swap(*this);
        }
        return *this;
      }

      template<class U>
      IntrusivePtr &operator=(const IntrusivePtr<U> &other) noexcept {
        IntrusivePtr(other).swap(*this);
        return *this;
      }

      IntrusivePtr &operator=(IntrusivePtr &&other) noexcept {
        IntrusivePtr(static_cast<IntrusivePtr &&>(other)).swap(*this);
        return *this;
      }

      template<class U>
      IntrusivePtr &operator=(IntrusivePtr<U> &&other) noexcept {
        IntrusivePtr(static_cast<IntrusivePtr<U> &&>(other)).swap(*this);
        return *this;
      }

      void swap(IntrusivePtr &&r) noexcept {
        T *tmp = instance;
        instance = r.instance;
        r.instance = tmp;
      }

      void swap(IntrusivePtr &r) noexcept {
        T *tmp = instance;
        instance = r.instance;
        r.instance = tmp;
      }

      [[nodiscard]] T *get() const noexcept {
        return instance;
      }

      operator T *() const {
        return instance;
      }

      InterfaceType *operator->() const noexcept {
        return instance;
      }

      T **operator&() // NOLINT(google-runtime-operator)
      {
        return releaseAndGetAddressOf();
      }

      [[nodiscard]] T *const*getAddressOf() const noexcept {
        return &instance;
      }

      [[nodiscard]] T **getAddressOf() noexcept {
        return &instance;
      }

      [[nodiscard]] T **releaseAndGetAddressOf() noexcept {
        internalRelease();
        return &instance;
      }

      T *detach() noexcept {
        T *ptr = instance;
        instance = nullptr;
        return ptr;
      }

      // Set the pointer while keeping the object's reference count unchanged
      void attach(InterfaceType *other) {
        if (instance != nullptr) {
          auto ref = instance->release();
          (void) ref;

          // attaching to the same object only works if duplicate references are being coalesced. Otherwise
          // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
          assert(ref != 0 || instance != other);
        }

        instance = other;
      }

      // Create a wrapper around a raw object while keeping the object's reference count unchanged
      template <typename... Args>
      static IntrusivePtr<T> create(Args &&... args) {
        return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
      }

      unsigned long reset() {
        return internalRelease();
      }
  };
}
