#pragma once
#include <algorithm>
#include <utility>

namespace Core::Context::detail {
  template<typename T, typename U = T>
  T exchange(T &t, U &&nv) {
    T ov = std::move(t);
    t = std::forward<U>(nv);
    return ov;
  }
}
