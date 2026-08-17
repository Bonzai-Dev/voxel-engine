#pragma once

#include <algorithm>
#include <utility>

// #include <boost/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename T, typename U = T>
      T exchange(T &t, U &&nv) {
        T ov = std::move(t);
        t = std::forward<U>(nv);
        return ov;
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #include BOOST_ABI_SUFFIX
// #endif
