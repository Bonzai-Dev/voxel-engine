//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <algorithm>
#include <utility>
// #include <boost/config.hpp>

namespace boost::context::detail {
  template <typename T, typename U = T>
  T exchange(T &t, U &&nv) {
    T ov = std::move(t);
    t = std::forward<U>(nv);
    return ov;
  }
}
