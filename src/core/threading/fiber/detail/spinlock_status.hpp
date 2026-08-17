#pragma once

namespace Core {
  namespace Fibers {
    namespace detail {
      enum class spinlock_status {
        locked = 0,
        unlocked
      };
    }
  }
}
