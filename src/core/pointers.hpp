#pragma once
#include <thread>

namespace Core {
  // Class that is used as a base class for resources that needs reference counting
  class RefCounted {
    public:
      RefCounted() = default;

      virtual ~RefCounted() = default;

      void incrementReference() const {
        ++referenceCount;
      }

      void decrementReference() const {
        --referenceCount;
      }

      unsigned long getReferenceCount() const {
        return referenceCount.load();
      }

    private:
      mutable std::atomic<unsigned long> referenceCount = 0;
  };

  template<typename T>
  class RefCountedPointer {
    public:
      RefCountedPointer() : instance(nullptr) {
      }

      RefCountedPointer(std::nullptr_t) : instance(nullptr) {
      }

      RefCountedPointer(T *other) : instance(other) {
        static_assert(std::is_base_of_v<RefCounted, T>, "Class does not inherit ReferenceCounted");
        increment();
      }

      RefCountedPointer(const RefCountedPointer<T> &other) : instance(other.instance) {
        increment();
      }

      // Copy constructor that allows to instantiate class when U* is convertible to T*
      template<typename U>
      requires(std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
      RefCountedPointer(const RefCountedPointer<U> &other) : instance(other.instance) {
        increment();
      }

      // Move constructor that allows to instantiate class when U* is convertible to T*
      template<typename U>
      RefCountedPointer(RefCountedPointer<U> &&other) : instance(other.instance) {
        other.instance = nullptr;
      }

      ~RefCountedPointer() {
        decrement();
      }

      RefCountedPointer &operator=(const RefCountedPointer<T> &other) {
        if (this == other)
          return *this;

        other.increment();
        decrement();
        instance = other.instance;
        return *this;
      }

      RefCountedPointer &operator=(RefCountedPointer<T> &&other)  noexcept {
        decrement();
        instance = other.instance;
        other.instance = nullptr;
        return *this;
      }

      // Assignment constructors when U is convertible to T
      template<typename U>
      RefCountedPointer &operator=(const RefCountedPointer<U> &other) {
        other.increment();
        decrement();
        instance = other.instance;
        return *this;
      }

      template<typename U>
      RefCountedPointer &operator=(RefCountedPointer<U> &&other) {
        decrement();
        instance = other.instance;
        other.instance = nullptr;
        return *this;
      }

      unsigned long getReferenceCount() const {
        return instance->getReferenceCount();
      }

      bool operator==(const RefCountedPointer<T> & other) const {
        return instance == other.instance;
      }

      bool operator!=(const RefCountedPointer<T> & other) const {
        return !(instance == other.instance);
      }

      T *operator->() { return instance; }
      const T *operator->() const { return instance; }

      T &operator*() { return *instance; }
      const T &operator*() const { return *instance; }

      T *get() { return instance; }
      const T *get() const { return instance; }

      void reset(T *instance = nullptr) {
        decrement();
        this->instance = instance;
      }

      template<typename ... Args>
      static RefCountedPointer<T> create(Args &&... args) {
        return RefCountedPointer<T>(new T(std::forward<Args>(args)...));
      }

    private:
      void increment() const {
        if (instance)
          instance->incrementReference();
      }

      void decrement() const {
        if (instance) {
          instance->decrementReference();
          if (instance->getReferenceCount() == 0) {
            delete instance;
            instance = nullptr;
          }
        }
      }

      template<class U>
      friend class RefCountedPointer;

      mutable T *instance = nullptr;
  };
}