#pragma once

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <cmath>
#include <cstddef>
#include <new>
#include <core/assert.hpp>
#include <boost/core/ignore_unused.hpp>
#include "../detail/config.hpp"
#include "../stack_context.hpp"
#include "../stack_traits.hpp"

#if defined(BOOST_USE_VALGRIND)
#include <valgrind/valgrind.h>
#endif

namespace Core::Context {
  template<typename traitsT>
  class basic_protected_fixedsize_stack {
    public:
      typedef traitsT traits_type;

      basic_protected_fixedsize_stack(std::size_t size = traits_type::default_size())

      noexcept: size_(size) {
      }

      stack_context allocate() {
        // calculate how many pages are required
        const std::size_t pages = (size_ + traits_type::page_size() - 1) / traits_type::page_size();
        // add one page at bottom that will be used as guard-page
        const std::size_t size__ = (pages + 1) * traits_type::page_size();

#if defined(BOOST_CONTEXT_USE_MAP_STACK)
        void *vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_STACK, -1, 0);
#elif defined(MAP_ANON)
        void *vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
        void *vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
        if (MAP_FAILED == vp) throw std::bad_alloc();

        // conforming to POSIX.1-2001
        const int result(::mprotect(vp, traits_type::page_size(), PROT_NONE));
        Core::ignore_unused(result);
        ENGINE_ASSERT(0 == result, "");

        stack_context sctx;
        sctx.size = size__;
        sctx.sp = static_cast<char *>(vp) + sctx.size;
#if defined(BOOST_USE_VALGRIND)
        sctx.valgrind_stack_id = VALGRIND_STACK_REGISTER(sctx.sp, vp);
#endif
        return sctx;
      }

      void deallocate(stack_context &sctx) noexcept {
        ENGINE_ASSERT(sctx.sp, "");

#if defined(BOOST_USE_VALGRIND)
        VALGRIND_STACK_DEREGISTER(sctx.valgrind_stack_id);
#endif

        void *vp = static_cast<char *>(sctx.sp) - sctx.size;
        // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
        ::munmap(vp, sctx.size);
      }

    private:
      std::size_t size_;
  };

  typedef basic_protected_fixedsize_stack<stack_traits> protected_fixedsize_stack;
}
