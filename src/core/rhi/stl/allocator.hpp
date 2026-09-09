// Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// Temporary allocator as we currently don't have our own custom allocator

#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <core/assert.hpp>
#include "creation.hpp"

#define NRI_MAX_STACK_ALLOC_SIZE 32768u // 32 Kb

// Helpers
template <typename T>
inline T Align(T x, size_t alignment) {
  return (T)((size_t(x) + alignment - 1) & ~(alignment - 1));
}

// clang-format off
#define NRI_ALLOCATE_SCRATCH(device, T, elementNum) { \
(device).allocationCallbacks, \
!(elementNum) ? nullptr : ( \
((elementNum) * sizeof(T) + alignof(T)) > NRI_MAX_STACK_ALLOC_SIZE \
? (T*)(device).allocationCallbacks.allocate((device).allocationCallbacks.userArg, (elementNum) * sizeof(T), alignof(T)) \
: (T*)Align((T*)alloca((elementNum) * sizeof(T) + alignof(T)), alignof(T)) \
), \
(elementNum) \
}
// clang-format on

namespace Core::RHI {
  struct AllocationCallbacks;
  template <typename T>
  struct StdAllocator {
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::false_type is_always_equal;

    StdAllocator(const AllocationCallbacks &allocationCallbacks): m_Interface(allocationCallbacks) {
    }

    StdAllocator(const StdAllocator<T> &allocator): m_Interface(allocator.GetInterface()) {
    }

    template <class U>
    StdAllocator(const StdAllocator<U> &allocator): m_Interface(allocator.GetInterface()) {
    }

    StdAllocator<T> &operator=(const StdAllocator<T> &allocator) {
      m_Interface = allocator.GetInterface();
      return *this;
    }

    T *allocate(size_t n) noexcept {
      return (T*)m_Interface.allocate(m_Interface.userArg, n * sizeof(T), alignof(T));
    }

    void deallocate(T *memory, size_t) noexcept {
      m_Interface.free(m_Interface.userArg, memory);
    }

    const AllocationCallbacks &GetInterface() const {
      return m_Interface;
    }

    template <typename U>
    using other = StdAllocator<U>;

    private:
      const AllocationCallbacks &m_Interface = {}; // IMPORTANT: yes, it's a pointer to the real location (DeviceBase)
  };

  template <typename T>
  bool operator==(const StdAllocator<T> &left, const StdAllocator<T> &right) {
    return left.GetInterface() == right.GetInterface();
  }

  template <typename T>
  bool operator!=(const StdAllocator<T> &left, const StdAllocator<T> &right) {
    return !operator==(left, right);
  }

  // Types with "StdAllocator"
  template <typename T>
  using Vector = std::vector<T, StdAllocator<T>>;

  template <typename U, typename T>
  using UnorderedMap = std::unordered_map<U, T, std::hash<U>, std::equal_to<U>, StdAllocator<std::pair<const U, T>>>;

  template <typename U, typename T>
  using Map = std::map<U, T, std::less<U>, StdAllocator<std::pair<const U, T>>>;

  using String = std::basic_string<char, std::char_traits<char>, StdAllocator<char>>;

  template <typename T, typename... Args>
  inline T* allocate(const AllocationCallbacks& allocationCallbacks, Args&&... args) {
    T* object = (T*)allocationCallbacks.allocate(allocationCallbacks.userArg, sizeof(T), alignof(T));
    if (object)
      new (object) T(std::forward<Args>(args)...);

    return object;
  }

  template <typename T>
  inline void destroy(const AllocationCallbacks& allocationCallbacks, T* object) {
    if (object) {
      object->~T();
      allocationCallbacks.free(allocationCallbacks.userArg, object);
    }
  }

  // Scratch
  template <typename T>
  class Scratch {
    public:
      Scratch(const AllocationCallbacks &allocator, T *mem, size_t num)
              : m_Allocator(allocator)
                , m_Mem(mem)
                , m_Num(num) {
        m_IsHeap = (num * sizeof(T) + alignof(T)) > NRI_MAX_STACK_ALLOC_SIZE;
      }

      ~Scratch() {
        if (m_IsHeap)
          m_Allocator.free(m_Allocator.userArg, m_Mem);
      }

      inline operator T*() const {
        return m_Mem;
      }

      inline T &operator[](size_t i) const {
        ENGINE_ASSERT(i < m_Num, "Out of bounds");
        return m_Mem[i];
      }

    private:
      const AllocationCallbacks &m_Allocator;
      T *m_Mem = nullptr;
      size_t m_Num = 0;
      bool m_IsHeap = false;
  };


}
