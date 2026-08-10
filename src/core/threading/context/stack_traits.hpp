#pragma once
#include <cstddef>
#include "detail/config.hpp"

namespace Core::Context {
  struct stack_traits {
    static bool is_unbounded() noexcept;

    static std::size_t page_size() noexcept;

    static std::size_t default_size() noexcept;

    static std::size_t minimum_size() noexcept;

    static std::size_t maximum_size() noexcept;
  };
}
