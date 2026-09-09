//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
extern "C" {
#include <windows.h>
}

#include <cmath>
#include <cstddef>
#include <new>

#include <core/assert.hpp>
// #include <boost/config.hpp>

#include <core/threading/context/detail/config.hpp>
#include <core/threading/context/stack_context.hpp>
#include <core/threading/context/stack_traits.hpp>

namespace boost::context {
  template <typename traitsT>
  class basic_protected_fixedsize_stack {
    private:
      std::size_t size_;

    public:
      typedef traitsT traits_type;

      basic_protected_fixedsize_stack(std::size_t size = traits_type::default_size()) noexcept :
        size_(size) {
      }

      stack_context allocate() {
        // calculate how many pages are required
        const std::size_t pages = (size_ + traits_type::page_size() - 1) / traits_type::page_size();
        // add one page at bottom that will be used as guard-page
        const std::size_t size__ = (pages + 1) * traits_type::page_size();

        void *vp = ::VirtualAlloc(0, size__, MEM_COMMIT, PAGE_READWRITE);
        if (!vp) throw std::bad_alloc();

        DWORD old_options;
        [[maybe_unused]] const BOOL result = ::VirtualProtect(
          vp,
          traits_type::page_size(),
          PAGE_READWRITE | PAGE_GUARD /*PAGE_NOACCESS*/,
          &old_options
        );
        ENGINE_ASSERT(FALSE != result, "");

        stack_context sctx;
        sctx.size = size__;
        sctx.sp = static_cast<char*>(vp) + sctx.size;
        return sctx;
      }

      void deallocate(stack_context &sctx) noexcept {
        ENGINE_ASSERT(sctx.sp, "");

        void *vp = static_cast<char*>(sctx.sp) - sctx.size;
        ::VirtualFree(vp, 0, MEM_RELEASE);
      }
  };

  typedef basic_protected_fixedsize_stack<stack_traits> protected_fixedsize_stack;
}
