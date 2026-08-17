#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

#include <core/assert.hpp>
// #include <boost/config.hpp>

#include <core/threading/context/detail/config.hpp>
#include <core/threading/context/stack_context.hpp>
#include <core/threading/context/stack_traits.hpp>

#if defined(BOOST_CONTEXT_USE_MAP_STACK)
extern "C" {
#include <sys/mman.h>
}
#endif

#if defined(BOOST_USE_VALGRIND)
#include <valgrind/valgrind.h>
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core::Context {
  template <typename traitsT>
  class basic_fixedsize_stack {
    private:
      std::size_t size_;

    public:
      typedef traitsT traits_type;

      basic_fixedsize_stack(std::size_t size = traits_type::default_size()) noexcept:
        size_(size) {
      }

      stack_context allocate() {
#if defined(BOOST_CONTEXT_USE_MAP_STACK)
        void *vp = ::mmap(0, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_STACK, -1, 0);
        if (vp == MAP_FAILED) {
          throw std::bad_alloc();
        }
#else
        void *vp = std::malloc(size_);
        if (!vp) {
          throw std::bad_alloc();
        }
#endif
        stack_context sctx;
        sctx.size = size_;
        sctx.sp = static_cast<char*>(vp) + sctx.size;
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
        void *vp = static_cast<char*>(sctx.sp) - sctx.size;
#if defined(BOOST_CONTEXT_USE_MAP_STACK)
        ::munmap(vp, sctx.size);
#else
        std::free(vp);
#endif
      }
  };

  typedef basic_fixedsize_stack<stack_traits> fixedsize_stack;
# if ! defined(BOOST_USE_SEGMENTED_STACKS)
  typedef fixedsize_stack default_stack;
# endif
}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
