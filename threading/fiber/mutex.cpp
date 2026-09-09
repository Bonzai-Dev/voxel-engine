//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "mutex.hpp"

#include <algorithm>
#include <functional>
#include <system_error>

#include "context.hpp"
#include "exceptions.hpp"
#include "scheduler.hpp"
#include "waker.hpp"
#include "detail/spinlock.hpp"

namespace boost {
  namespace fibers {
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
