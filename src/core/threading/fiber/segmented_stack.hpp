#pragma once
// #include <boost/config.hpp>
#include <core/threading/context/segmented_stack.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
#if defined(BOOST_USE_SEGMENTED_STACKS)
#if !defined(ENGINE_PLATFORM_WINDOWS)
    using segmented_stack = Core::context::segmented_stack;
    using default_stack = Core::context::default_stack;
# endif
#endif
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
