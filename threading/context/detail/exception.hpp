//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/assert.hpp>
// #include <boost/config.hpp>
#include <core/threading/context/detail/fcontext.hpp>

namespace boost::context::detail {
  struct forced_unwind {
    fcontext_t fctx{nullptr};

    forced_unwind() = default;

    forced_unwind(fcontext_t fctx_) :
      fctx(fctx_) {
    }
  };
}
