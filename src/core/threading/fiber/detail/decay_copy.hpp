#pragma once

#include <type_traits>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename T>
      typename std::decay<T>::type decay_copy(T &&t) {
        return std::forward<T>(t);
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
