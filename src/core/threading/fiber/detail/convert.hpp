#pragma once

#include <chrono>
#include <memory>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      inline std::chrono::steady_clock::time_point convert(
        std::chrono::steady_clock::time_point const &timeout_time) noexcept {
        return timeout_time;
      }

      template <typename Clock, typename Duration>
      std::chrono::steady_clock::time_point convert(
        std::chrono::time_point<Clock, Duration> const &timeout_time) {
        return std::chrono::steady_clock::now() + (timeout_time - Clock::now());
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
