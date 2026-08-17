#pragma once
#include <type_traits>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core::Context::detail {
  // http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
  template <typename X, typename Y>
  using disable_overload =
  typename std::enable_if<
    !std::is_base_of<
      X,
      typename std::decay<Y>::type
    >::value
  >::type;
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #include BOOST_ABI_SUFFIX
// #endif
//