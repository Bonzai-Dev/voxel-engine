#pragma once
#include <type_traits>
#include "config.hpp"

namespace Core::Context::detail {
  // http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
  template<typename X, typename Y>
  using disable_overload =
  typename std::enable_if<
    !std::is_base_of<
      X,
      typename std::decay<Y>::type
    >::value
  >::type;
}
