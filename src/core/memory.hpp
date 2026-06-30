#pragma once
#include <cstdint>
#include <atomic>

namespace Core {
  class RefCounted {
    public:
      virtual ~RefCounted() = default;

      void incrementReferenceCount() const {
        ++referenceCount;
      }

      void decrementReferenceCount() const {
        --referenceCount;
      }

      uint32_t getReferenceCount() const { return referenceCount.load(); }

    private:
      mutable std::atomic<uint32_t> referenceCount = 0;
  };

  template <typename T>
  class RefCountedPtr {
    public:
      RefCountedPtr(): instance(nullptr) {
      }

      RefCountedPtr(std::nullptr_t): instance(nullptr) {
      }

      RefCountedPtr(T *instance): instance(instance) {
        static_assert(std::is_base_of<RefCounted, T>::value, "Class is not a base of RefCounted");
        increment();
      }

      template <typename U>
      RefCountedPtr(const RefCountedPtr<U> &other) {
        instance = (T*)other.instance;
        increment();
      }

      template <typename U>
      RefCountedPtr(RefCountedPtr<U> &&other) {
        instance = (T*)other.instance;
        other.instance = nullptr;
      }

      ~RefCountedPtr() {
        decrement();
      }

      RefCountedPtr(const RefCountedPtr<T> &other)
        : instance(other.instance) {
        increment();
      }

      RefCountedPtr &operator=(std::nullptr_t) {
        decrement();
        instance = nullptr;
        return *this;
      }

      RefCountedPtr &operator=(const RefCountedPtr<T> &other) {
        if (this == &other)
          return *this;

        other.increment();
        decrement();

        instance = other.instance;
        return *this;
      }

      template <typename U>
      RefCountedPtr &operator=(const RefCountedPtr<U> &other) {
        other.increment();
        decrement();

        instance = other.instance;
        return *this;
      }

      template <typename U>
      RefCountedPtr &operator=(RefCountedPtr<U> &&other) {
        decrement();

        instance = other.instance;
        other.instance = nullptr;
        return *this;
      }

      operator bool() { return instance != nullptr; }
      operator bool() const { return instance != nullptr; }

      T *operator->() { return instance; }
      const T *operator->() const { return instance; }

      T &operator*() { return *instance; }
      const T &operator*() const { return *instance; }

      T *raw() { return instance; }
      const T *raw() const { return instance; }

      void reset(T *other = nullptr) {
        decrement();
        instance = other;
      }

      template <typename U>
      RefCountedPtr<U> as() const {
        return RefCountedPtr<U>(*this);
      }

      template <typename... Args>
      static RefCountedPtr<T> create(Args &&... args) {
        return RefCountedPtr<T>(new T(std::forward<Args>(args)...));
      }

      bool operator==(const RefCountedPtr<T> &other) const {
        return instance == other.instance;
      }

      bool operator!=(const RefCountedPtr<T> &other) const {
        return !(*this == other);
      }

    private:
      void increment() const {
        if (instance) {
          instance->incrementReferenceCount();
        }
      }

      void decrement() const {
        if (instance) {
          instance->decrementReferenceCount();

          if (instance->getReferenceCount() == 0) {
            delete instance;
            instance = nullptr;
          }
        }
      }

      template <class U>
      friend class RefCountedPtr;
      mutable T *instance;
  };
}
