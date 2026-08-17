#pragma once

// #include <boost/config.hpp>
// #include <boost/cstdint.hpp>
#include <core/threading/context/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core::Context::detail {
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

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
