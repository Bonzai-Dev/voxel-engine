#pragma once
#include <cstddef>
#include "config.hpp"

#if defined(BOOST_CONTEXT_NO_CXX14_INTEGER_SEQUENCE)
#include <boost/mp11/integer_sequence.hpp>
#endif

namespace Core::Context::detail {
#if ! defined(BOOST_CONTEXT_NO_CXX14_INTEGER_SEQUENCE)
  template<std::size_t ... I>
  using index_sequence = std::index_sequence<I...>;
  template<std::size_t I>
  using make_index_sequence = std::make_index_sequence<I>;
  template<typename... T>
  using index_sequence_for = std::index_sequence_for<T...>;
#else
  template<std::size_t ... I>
  using index_sequence = mp11::index_sequence<I...>;
  template<std::size_t I>
  using make_index_sequence = mp11::make_index_sequence<I>;
  template<typename... T>
  using index_sequence_for = mp11::index_sequence_for<T...>;
#endif
}
