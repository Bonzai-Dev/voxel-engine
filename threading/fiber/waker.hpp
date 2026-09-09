//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>

// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>
// #include <boost/intrusive/slist.hpp>

namespace boost::fibers {
  class context;

  namespace detail {
    // typedef intrusive::slist_member_hook<> waker_queue_hook;
    typedef std::slist_member_hook<> waker_queue_hook;
  } // detail

  class waker {
    private:
      context *ctx_{};
      size_t epoch_{};

    public:
      friend class context;

      waker() = default;

      waker(context *ctx, const size_t epoch)
        : ctx_{ctx}
          , epoch_{epoch} {
      }

      bool wake() const noexcept;
  };

  class waker_with_hook: public waker {
    public:
      explicit waker_with_hook(waker &&w)
        : waker{std::move(w)} {
      }

      bool is_linked() const noexcept {
        return waker_queue_hook_.is_linked();
      }

      friend bool
      operator==(waker const &lhs, waker const &rhs) noexcept {
        return &lhs == &rhs;
      }

    public:
      detail::waker_queue_hook waker_queue_hook_{};
  };

  namespace detail {
    // typedef intrusive::slist<
    //   waker_with_hook,
    //   intrusive::member_hook<
    //     waker_with_hook, detail::waker_queue_hook, &waker_with_hook::waker_queue_hook_>,
    //   intrusive::constant_time_size<false>,
    //   intrusive::cache_last<true>
    // > waker_slist_t;

    typedef intrusive::slist<
      waker_with_hook,
      intrusive::member_hook<
        waker_with_hook, detail::waker_queue_hook, &waker_with_hook::waker_queue_hook_>,
      intrusive::constant_time_size<false>,
      intrusive::cache_last<true>
    > waker_slist_t;
  }

  class wait_queue {
    private:
      detail::waker_slist_t slist_{};

    public:
      void suspend_and_wait(detail::spinlock_lock &, context *);
      bool suspend_and_wait_until(detail::spinlock_lock &, context *, std::chrono::steady_clock::time_point const &);
      void notify_one();
      void notify_all();

      bool empty() const;
  };
}

