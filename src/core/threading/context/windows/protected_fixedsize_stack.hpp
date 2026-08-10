#pragma once

extern "C" {
#include <windows.h>
}

#include <cmath>
#include <cstddef>
#include <new>
#include <core/assert.hpp>
#include <boost/core/ignore_unused.hpp>
#include "../detail/config.hpp"
#include "../stack_context.hpp"
#include "../stack_traits.hpp"

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

        void *vp = ::VirtualAlloc(0, size__, MEM_COMMIT, PAGE_READWRITE);
        if (!vp) throw std::bad_alloc();

        DWORD old_options;
        const BOOL result = ::VirtualProtect(
          vp, traits_type::page_size(), PAGE_READWRITE | PAGE_GUARD /*PAGE_NOACCESS*/, &old_options);
        Core::ignore_unused(result);
        ENGINE_STATIC_ASSERT(FALSE != result);

        stack_context sctx;
        sctx.size = size__;
        sctx.sp = static_cast<char *>(vp) + sctx.size;
        return sctx;
      }

      void deallocate(stack_context &sctx) noexcept {
        ENGINE_ASSERT(sctx.sp, "");

        void *vp = static_cast<char *>(sctx.sp) - sctx.size;
        ::VirtualFree(vp, 0, MEM_RELEASE);
      }

    private:
      std::size_t size_;
  };

  typedef basic_protected_fixedsize_stack<stack_traits> protected_fixedsize_stack;
}
