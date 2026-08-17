#pragma once

#include <cstddef>
#include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core::Context {
  struct preallocated {
    void *sp;
    std::size_t size;
    stack_context sctx;

    preallocated(void *sp_, std::size_t size_, stack_context sctx_) noexcept:
      sp(sp_), size(size_), sctx(sctx_) {
    }
  };
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
