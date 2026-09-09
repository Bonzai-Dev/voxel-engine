//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/threading/fiber/recursive_mutex.hpp>

#include <algorithm>
#include <functional>
#include <core/threading/fiber/exceptions.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include "context.hpp"
#include "detail/spinlock.hpp"

namespace boost::fibers {
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
