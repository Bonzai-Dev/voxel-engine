#pragma once

// #include <boost/config.hpp>
#include <core/threading/context/protected_fixedsize_stack.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core:: Fibers {
  using protected_fixedsize_stack = Core::Context::protected_fixedsize_stack;
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
