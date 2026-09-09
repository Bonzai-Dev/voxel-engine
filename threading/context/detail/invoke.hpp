//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include <type_traits>
#include <utility>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

namespace boost::context::detail {
  template <typename Fn, typename... Args>
  typename std::enable_if<
    std::is_member_pointer<typename std::decay<Fn>::type>::value,
    typename std::result_of<Fn &&(Args &&...)>::type
  >::type invoke(Fn &&fn, Args &&... args) {
    return std::mem_fn(fn)(std::forward<Args>(args)...);
  }

  template <typename Fn, typename... Args>
  typename std::enable_if<
    !std::is_member_pointer<typename std::decay<Fn>::type>::value,
    typename std::result_of<Fn &&(Args &&...)>::type
  >::type invoke(Fn &&fn, Args &&... args) {
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }
}
