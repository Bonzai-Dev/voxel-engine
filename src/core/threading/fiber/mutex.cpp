#include "mutex.hpp"

#include <algorithm>
#include <functional>
#include <system_error>

#include <core/threading/fiber/exceptions.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include <core/threading/fiber/waker.hpp>
#include "detail/spinlock.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    void mutex::lock() {
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

    bool mutex::try_lock() {
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

    void mutex::unlock() {
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
