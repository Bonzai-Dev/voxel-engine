//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/threading/context/detail/fcontext.hpp>

extern "C"
boost::context::detail::transfer_t BOOST_CONTEXT_CALLDECL jump_fcontext(
  boost::context::detail::fcontext_t const to, void *vp
);

extern "C"
boost::context::detail::fcontext_t BOOST_CONTEXT_CALLDECL make_fcontext(
  void *sp, std::size_t size, void (*fn)(boost::context::detail::transfer_t)
);

// based on an idea of Giovanni Derreta
extern "C"
boost::context::detail::transfer_t BOOST_CONTEXT_CALLDECL ontop_fcontext(
  boost::context::detail::fcontext_t const to, void *vp,
  boost::context::detail::transfer_t (*fn)(boost::context::detail::transfer_t)
);

namespace boost::context::detail {
  transfer_t jump_fcontext(fcontext_t const to, void *vp) {
    return ::jump_fcontext(to, vp);
  }

  fcontext_t make_fcontext(void *sp, std::size_t size, void (*fn)(transfer_t)) {
    return ::make_fcontext(sp, size, fn);
  }

  transfer_t ontop_fcontext(fcontext_t const to, void *vp, transfer_t (*fn)(transfer_t)) {
    return ::ontop_fcontext(to, vp, fn);
  }
}
