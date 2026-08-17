#include "timed_mutex.hpp"
#include <algorithm>
#include <functional>
#include "exceptions.hpp"
#include "scheduler.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    bool timed_mutex::try_lock_until_(std::chrono::steady_clock::time_point const &timeout_time) noexcept {
      while (true) {
        if (std::chrono::steady_clock::now() > timeout_time) {
          return false;
        }
        context *active_ctx = context::active();
        detail::spinlock_lock lk{wait_queue_splk_};
        if (nullptr == owner_) {
          owner_ = active_ctx;
          return true;
        }
        if (!wait_queue_.suspend_and_wait_until(lk, active_ctx, timeout_time)) {
          return false;
        }
      }
    }

    void timed_mutex::lock() {
      while (true) {
        context *active_ctx = context::active();
        // store this fiber in order to be notified later
        detail::spinlock_lock lk{wait_queue_splk_};
        if (active_ctx == owner_) [[unlikely]] {
          throw lock_error{
            std::make_error_code(std::errc::resource_deadlock_would_occur),
            "boost fiber: a deadlock is detected"
          };
        }
        if (nullptr == owner_) {
          owner_ = active_ctx;
          return;
        }
        wait_queue_.suspend_and_wait(lk, active_ctx);
      }
    }

    bool timed_mutex::try_lock() {
      context *active_ctx = context::active();
      detail::spinlock_lock lk{wait_queue_splk_};
      if (active_ctx == owner_) [[unlikely]] {
        throw lock_error{
          std::make_error_code(std::errc::resource_deadlock_would_occur),
          "boost fiber: a deadlock is detected"
        };
      }
      if (nullptr == owner_) {
        owner_ = active_ctx;
      }
      lk.unlock();
      // let other fiber release the lock
      active_ctx->yield();
      return active_ctx == owner_;
    }

    void timed_mutex::unlock() {
      context *active_ctx = context::active();
      detail::spinlock_lock lk{wait_queue_splk_};
      if (active_ctx != owner_) [[unlikely]] {
        throw lock_error{
          std::make_error_code(std::errc::operation_not_permitted),
          "boost fiber: no  privilege to perform the operation"
        };
      }
      owner_ = nullptr;

      wait_queue_.notify_one();
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
