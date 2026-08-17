#include "recursive_mutex.hpp"
#include <algorithm>
#include <functional>
#include "context.hpp"
#include "exceptions.hpp"
#include "scheduler.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    void recursive_mutex::lock() {
      while (true) {
        context *active_ctx = context::active();
        // store this fiber in order to be notified later
        detail::spinlock_lock lk{wait_queue_splk_};
        if (active_ctx == owner_) {
          ++count_;
          return;
        }
        if (nullptr == owner_) {
          owner_ = active_ctx;
          count_ = 1;
          return;
        }

        wait_queue_.suspend_and_wait(lk, active_ctx);
      }
    }

    bool recursive_mutex::try_lock() noexcept {
      context *active_ctx = context::active();
      detail::spinlock_lock lk{wait_queue_splk_};
      if (nullptr == owner_) {
        owner_ = active_ctx;
        count_ = 1;
      }
      else if (active_ctx == owner_) {
        ++count_;
      }
      lk.unlock();
      // let other fiber release the lock
      context::active()->yield();
      return active_ctx == owner_;
    }

    void recursive_mutex::unlock() {
      context *active_ctx = context::active();
      detail::spinlock_lock lk(wait_queue_splk_);
      if (active_ctx != owner_) [[unlikely]] {
        throw lock_error(
          std::make_error_code(std::errc::operation_not_permitted),
          "boost fiber: no  privilege to perform the operation");
      }
      if (0 == --count_) {
        owner_ = nullptr;
        wait_queue_.notify_one();
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
