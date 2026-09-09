//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "condition_variable.hpp"
#include "context.hpp"
#include "detail/spinlock.hpp"

namespace boost::fibers {
  void condition_variable_any::notify_one() noexcept {
    detail::spinlock_lock lk{wait_queue_splk_};
    wait_queue_.notify_one();
  }

  void condition_variable_any::notify_all() noexcept {
    detail::spinlock_lock lk{wait_queue_splk_};
    wait_queue_.notify_all();
  }
}
