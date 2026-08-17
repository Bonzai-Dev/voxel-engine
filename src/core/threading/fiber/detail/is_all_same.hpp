#pragma once

#include <type_traits>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename X, typename... Y>
      struct is_all_same;

      template <typename X, typename Y0, typename... Y>
      struct is_all_same<X, Y0, Y...> {
        static constexpr bool value =
          std::is_same<X, Y0>::value && is_all_same<X, Y...>::value;
      };

      template <typename X, typename Y0>
      struct is_all_same<X, Y0> {
        static constexpr bool value = std::is_same<X, Y0>::value;
      };
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
