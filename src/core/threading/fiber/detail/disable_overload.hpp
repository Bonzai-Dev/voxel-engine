#pragma once

#include <type_traits>

// #include <boost/config.hpp>
#include <core/threading/context/detail/disable_overload.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename X, typename Y>
      using disable_overload = Core::Context::detail::disable_overload<X, Y>;
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #include BOOST_ABI_SUFFIX
// #endif
