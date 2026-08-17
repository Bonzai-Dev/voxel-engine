#pragma once

#include <future>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

namespace Core {
  namespace Fibers {
    enum class future_status {
      ready = 1,
      timeout,
      deferred
    };
  }
}
