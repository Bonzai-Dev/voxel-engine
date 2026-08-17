#pragma once
// #include <boost/config.hpp>
#include <core/threading/context/pooled_fixedsize_stack.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    using pooled_fixedsize_stack = Core::Context::pooled_fixedsize_stack;
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
