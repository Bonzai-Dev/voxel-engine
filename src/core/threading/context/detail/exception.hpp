#pragma once
#include <core/assert.hpp>
//#include <boost/config.hpp>
#include <core/threading/context/detail/fcontext.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core::Context::detail {
  struct forced_unwind {
    fcontext_t fctx{nullptr};

    forced_unwind() = default;

    forced_unwind(fcontext_t fctx_):
      fctx(fctx_) {
    }
  };
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #include BOOST_ABI_SUFFIX
// #endif
