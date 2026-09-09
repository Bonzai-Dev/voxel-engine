//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <cstdint>
// #include <boost/config.hpp>
// #include <boost/predef.h>
#include <core/threading/context/detail/config.hpp>

#include "core/application.hpp"

#if ENGINE_COMPILER_MSVC && !defined(_M_ARM) && !defined(_M_ARM64)
#include <mmintrin.h>
#endif

namespace boost::context::detail {
#if ENGINE_COMPILER_GCC || ENGINE_COMPILER_CLANG
#define BOOST_HAS_PREFETCH 1
  ENGINE_FORCE_INLINE void prefetch(void *addr) {
    // L1 cache : hint == 1
    __builtin_prefetch(addr, 1, 1);
  }
#elif ENGINE_COMPILER_MSVC && !defined(_M_ARM) && !defined(_M_ARM64)
#define BOOST_HAS_PREFETCH 1
  ENGINE_FORCE_INLINE void prefetch(void *addr) {
    // L1 cache : hint == _MM_HINT_T0
    _mm_prefetch((const char*)addr, _MM_HINT_T0);
  }
#endif

  inline void prefetch_range(void *addr, std::size_t len) {
#if defined(BOOST_HAS_PREFETCH)
    void *vp = addr;
    void *end = reinterpret_cast<void*>(
      reinterpret_cast<uintptr_t>(addr) + static_cast<uintptr_t>(len));
    while (vp < end) {
      prefetch(vp);
      vp = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(vp) + static_cast<uintptr_t>(prefetch_stride));
    }
#endif
  }

#undef BOOST_HAS_PREFETCH
}
