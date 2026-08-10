// #pragma once
// #include <cstdint>
// #include <atomic>
//
// namespace Core {
//   class RefCounted {
//     public:
//       virtual ~RefCounted() = default;
//
//       void incrementReferenceCount() const {
//         ++referenceCount;
//       }
//
//       void decrementReferenceCount() const {
//         --referenceCount;
//       }
//
//       uint32_t getReferenceCount() const { return referenceCount.load(); }
//
//     private:
//       mutable std::atomic<uint32_t> referenceCount = 0;
//   };
//
//   template <typename T>
//   class RefCountedPtr {
//     public:
//       RefCountedPtr(): instance(nullptr) {
//       }
//
//       RefCountedPtr(std::nullptr_t): instance(nullptr) {
//       }
//
//       RefCountedPtr(T *instance): instance(instance) {
//         static_assert(std::is_base_of<RefCounted, T>::value, "Class is not a base of RefCounted");
//         increment();
//       }
//
//       template <typename U>
//       RefCountedPtr(const RefCountedPtr<U> &other) {
//         instance = (T*)other.instance;
//         increment();
//       }
//
//       template <typename U>
//       RefCountedPtr(RefCountedPtr<U> &&other) {
//         instance = (T*)other.instance;
//         other.instance = nullptr;
//       }
//
//       ~RefCountedPtr() {
//         decrement();
//       }
//
//       RefCountedPtr(const RefCountedPtr<T> &other)
//         : instance(other.instance) {
//         increment();
//       }
//
//       RefCountedPtr &operator=(std::nullptr_t) {
//         decrement();
//         instance = nullptr;
//         return *this;
//       }
//
//       RefCountedPtr &operator=(const RefCountedPtr<T> &other) {
//         if (this == &other)
//           return *this;
//
//         other.increment();
//         decrement();
//
//         instance = other.instance;
//         return *this;
//       }
//
//       template <typename U>
//       RefCountedPtr &operator=(const RefCountedPtr<U> &other) {
//         other.increment();
//         decrement();
//
//         instance = other.instance;
//         return *this;
//       }
//
//       template <typename U>
//       RefCountedPtr &operator=(RefCountedPtr<U> &&other) {
//         decrement();
//
//         instance = other.instance;
//         other.instance = nullptr;
//         return *this;
//       }
//
//       operator bool() { return instance != nullptr; }
//       operator bool() const { return instance != nullptr; }
//
//       T *operator->() { return instance; }
//       const T *operator->() const { return instance; }
//
//       T &operator*() { return *instance; }
//       const T &operator*() const { return *instance; }
//
//       T *raw() { return instance; }
//       const T *raw() const { return instance; }
//
//       void reset(T *other = nullptr) {
//         decrement();
//         instance = other;
//       }
//
//       template <typename U>
//       RefCountedPtr<U> as() const {
//         return RefCountedPtr<U>(*this);
//       }
//
//       template <typename... Args>
//       static RefCountedPtr<T> create(Args &&... args) {
//         return RefCountedPtr<T>(new T(std::forward<Args>(args)...));
//       }
//
//       bool operator==(const RefCountedPtr<T> &other) const {
//         return instance == other.instance;
//       }
//
//       bool operator!=(const RefCountedPtr<T> &other) const {
//         return !(*this == other);
//       }
//
//     private:
//       void increment() const {
//         if (instance) {
//           instance->incrementReferenceCount();
//         }
//       }
//
//       void decrement() const {
//         if (instance) {
//           instance->decrementReferenceCount();
//
//           if (instance->getReferenceCount() == 0) {
//             delete instance;
//             instance = nullptr;
//           }
//         }
//       }
//
//       template <class U>
//       friend class RefCountedPtr;
//       mutable T *instance;
//   };
// }

#pragma once
#include <cstdint>
#include <atomic>

namespace Core {
  //////////////////////////////////////////////////////////////////////////
  // RefCounted
  // A class that implements reference counting in a way compatible with RefCountPtr.
  // Intended usage is to use it as a base class for interface implementations, like so:
  // class Texture : public RefCounted { ... }
  //////////////////////////////////////////////////////////////////////////

  class RefCounted {
    public:
      virtual ~RefCounted() = default;

      virtual unsigned long addRef() {
        return ++m_refCount;
      }

      virtual unsigned long release() {
        unsigned long result = --m_refCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }

      virtual unsigned long getRefCount() {
        return m_refCount.load();
      }

    private:
      mutable std::atomic<unsigned long> m_refCount = 1;
  };

  //////////////////////////////////////////////////////////////////////////
  // RefCountPtr
  // Mostly a copy of Microsoft::WRL::ComPtr<T>
  //////////////////////////////////////////////////////////////////////////
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
      InterfaceType *ptr_;
      template<class U>
      friend class RefCountedPtr;

      void internalAddRef() const noexcept {
        if (ptr_ != nullptr) {
          ptr_->addRef();
        }
      }

      unsigned long internalRelease() noexcept {
        unsigned long ref = 0;
        T *temp = ptr_;

        if (temp != nullptr) {
          ptr_ = nullptr;
          ref = temp->release();
        }

        return ref;
      }

    public:
      RefCountedPtr() noexcept: ptr_(nullptr) {
      }

      RefCountedPtr(std::nullptr_t) noexcept: ptr_(nullptr) {
      }

      template<class U>
      RefCountedPtr(U *other) noexcept: ptr_(other) {
        internalAddRef();
      }

      RefCountedPtr(const RefCountedPtr &other) noexcept: ptr_(other.ptr_) {
        internalAddRef();
      }

      // copy ctor that allows to instantiate class when U* is convertible to T*
      template<class U>
      RefCountedPtr(const RefCountedPtr<U> &other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: ptr_(
        other.ptr_) {
        internalAddRef();
      }

      RefCountedPtr(RefCountedPtr &&other) noexcept: ptr_(nullptr) {
        if (this != reinterpret_cast<RefCountedPtr *>(&reinterpret_cast<unsigned char &>(other))) {
          swap(other);
        }
      }

      // Move ctor that allows instantiation of a class when U* is convertible to T*
      template<class U>
      RefCountedPtr(RefCountedPtr<U> &&other, std::enable_if_t<std::is_convertible_v<U *, T *>, void *> * = nullptr)
        noexcept: ptr_(
        other.ptr_) {
        other.ptr_ = nullptr;
      }

      ~RefCountedPtr() noexcept {
        internalRelease();
      }

      RefCountedPtr &operator=(std::nullptr_t) noexcept {
        internalRelease();
        return *this;
      }

      RefCountedPtr &operator=(T *other) noexcept {
        if (ptr_ != other) {
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
        if (ptr_ != other.ptr_) {
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
        T *tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
      }

      void swap(RefCountedPtr &r) noexcept {
        T *tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
      }

      [[nodiscard]] T *get() const noexcept {
        return ptr_;
      }

      operator T *() const {
        return ptr_;
      }

      InterfaceType *operator->() const noexcept {
        return ptr_;
      }

      T **operator&() // NOLINT(google-runtime-operator)
      {
        return releaseAndGetAddressOf();
      }

      [[nodiscard]] T *const*getAddressOf() const noexcept {
        return &ptr_;
      }

      [[nodiscard]] T **getAddressOf() noexcept {
        return &ptr_;
      }

      [[nodiscard]] T **releaseAndGetAddressOf() noexcept {
        internalRelease();
        return &ptr_;
      }

      T *detach() noexcept {
        T *ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
      }

      // Set the pointer while keeping the object's reference count unchanged
      void attach(InterfaceType *other) {
        if (ptr_ != nullptr) {
          auto ref = ptr_->release();
          (void) ref;

          // attaching to the same object only works if duplicate references are being coalesced. Otherwise
          // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
          assert(ref != 0 || ptr_ != other);
        }

        ptr_ = other;
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
