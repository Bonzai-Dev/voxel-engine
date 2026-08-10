#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include "config.hpp"

namespace Core::Context::detail {
  template<typename Fn, typename... Args>
  typename std::enable_if<
    std::is_member_pointer<typename std::decay<Fn>::type>::value,
    typename std::result_of<Fn &&(Args &&...)>::type
  >::type invoke(Fn &&fn, Args &&... args) {
    return std::mem_fn(fn)(std::forward<Args>(args)...);
  }

  template<typename Fn, typename... Args>
  typename std::enable_if<
    !std::is_member_pointer<typename std::decay<Fn>::type>::value,
    typename std::result_of<Fn &&(Args &&...)>::type
  >::type invoke(Fn &&fn, Args &&... args) {
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
  }
}
