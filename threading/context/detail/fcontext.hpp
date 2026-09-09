//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
// #include <boost/config.hpp>
// #include <boost/cstdint.hpp>

#include <cstddef>
#include <core/threading/context/detail/config.hpp>

namespace boost::context::detail {
  typedef void *fcontext_t;

  struct transfer_t {
    fcontext_t fctx;
    void *data;
  };

  transfer_t jump_fcontext(fcontext_t const to, void *vp);
  fcontext_t make_fcontext(void *sp, std::size_t size, void (*fn)(transfer_t));

  // based on an idea of Giovanni Derreta
  transfer_t ontop_fcontext(fcontext_t const to, void *vp, transfer_t (*fn)(transfer_t));
}
