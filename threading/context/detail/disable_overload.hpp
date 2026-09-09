//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

namespace boost::context::detail {
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
