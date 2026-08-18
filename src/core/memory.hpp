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
  class RefCountedPtr {
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
      friend class RefCountedPtr;

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
      RefCountedPtr() noexcept: instance(nullptr) {
      }

      RefCountedPtr(std::nullptr_t) noexcept: instance(nullptr) {
      }

      template<class U>
      RefCountedPtr(U *other) noexcept: instance(other) {
        internalAddRef();
      }

      RefCountedPtr(const RefCountedPtr &other) noexcept: instance(other.instance) {
        internalAddRef();
      }

      // copy ctor that allows to instantiate class when U* is convertible to T*
      template<class U>
      RefCountedPtr(const RefCountedPtr<U> &other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: instance(
        other.instance) {
        internalAddRef();
      }

      RefCountedPtr(RefCountedPtr &&other) noexcept: instance(nullptr) {
        if (this != reinterpret_cast<RefCountedPtr *>(&reinterpret_cast<unsigned char &>(other))) {
          swap(other);
        }
      }

      // Move ctor that allows instantiation of a class when U* is convertible to T*
      template<class U>
      RefCountedPtr(RefCountedPtr<U> &&other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: instance(
        other.instance) {
        other.instance = nullptr;
      }

      ~RefCountedPtr() noexcept {
        internalRelease();
      }

      RefCountedPtr &operator=(std::nullptr_t) noexcept {
        internalRelease();
        return *this;
      }

      RefCountedPtr &operator=(T *other) noexcept {
        if (instance != other) {
          RefCountedPtr(other).swap(*this);
        }
        return *this;
      }

      template<typename U>
      RefCountedPtr &operator=(U *other) noexcept {
        RefCountedPtr(other).swap(*this);
        return *this;
      }

      RefCountedPtr &operator=(const RefCountedPtr &other) noexcept // NOLINT(bugprone-unhandled-self-assignment)
      {
        if (instance != other.instance) {
          RefCountedPtr(other).swap(*this);
        }
        return *this;
      }

      template<class U>
      RefCountedPtr &operator=(const RefCountedPtr<U> &other) noexcept {
        RefCountedPtr(other).swap(*this);
        return *this;
      }

      RefCountedPtr &operator=(RefCountedPtr &&other) noexcept {
        RefCountedPtr(static_cast<RefCountedPtr &&>(other)).swap(*this);
        return *this;
      }

      template<class U>
      RefCountedPtr &operator=(RefCountedPtr<U> &&other) noexcept {
        RefCountedPtr(static_cast<RefCountedPtr<U> &&>(other)).swap(*this);
        return *this;
      }

      void swap(RefCountedPtr &&r) noexcept {
        T *tmp = instance;
        instance = r.instance;
        r.instance = tmp;
      }

      void swap(RefCountedPtr &r) noexcept {
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
      static RefCountedPtr<T> create(Args &&... args) {
        return RefCountedPtr<T>(new T(std::forward<Args>(args)...));
      }

      unsigned long reset() {
        return internalRelease();
      }
  };
}
