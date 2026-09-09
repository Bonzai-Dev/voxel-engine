//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>

// #include <boost/config.hpp>
// #include <boost/context/detail/config.hpp>

namespace boost::context::detail {
  template <std::size_t ... I>
  using index_sequence = std::index_sequence<I...>;
  template <std::size_t I>
  using make_index_sequence = std::make_index_sequence<I>;
  template <typename... T>
  using index_sequence_for = std::index_sequence_for<T...>;
}
