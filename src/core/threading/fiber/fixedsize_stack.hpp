#pragma once

// #include <boost/config.hpp>
#include <core/threading/context/fixedsize_stack.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    using fixedsize_stack = Core::Context::fixedsize_stack;
#if !defined(BOOST_USE_SEGMENTED_STACKS)
    using default_stack = Core::Context::default_stack;
#endif
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
