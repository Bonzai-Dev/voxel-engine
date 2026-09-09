//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

namespace boost::context {
  struct preallocated {
    void *sp;
    std::size_t size;
    stack_context sctx;

    preallocated(void *sp_, std::size_t size_, stack_context sctx_) noexcept :
      sp(sp_), size(size_), sctx(sctx_) {
    }
  };
}
