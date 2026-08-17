#include <core/threading/fiber/condition_variable.hpp>
#include <core/threading/fiber/context.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    void condition_variable_any::notify_one() noexcept {
      detail::spinlock_lock lk{wait_queue_splk_};
      wait_queue_.notify_one();
    }

    void condition_variable_any::notify_all() noexcept {
      detail::spinlock_lock lk{wait_queue_splk_};
      wait_queue_.notify_all();
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
