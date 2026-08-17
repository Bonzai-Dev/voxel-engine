#pragma once

#include <chrono>
#include <cstddef>
// #include <boost/config.hpp>
#include <core/assert.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/convert.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>
#include <core/threading/fiber/waker.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace Core {
  namespace Fibers {
    class condition_variable;

    class recursive_timed_mutex {
      private:
        friend class condition_variable;

        detail::spinlock wait_queue_splk_{};
        wait_queue wait_queue_{};
        context *owner_{nullptr};
        std::size_t count_{0};

        bool try_lock_until_(std::chrono::steady_clock::time_point const &timeout_time) noexcept;

      public:
        recursive_timed_mutex() = default;

        ~recursive_timed_mutex() {
          ENGINE_ASSERT(nullptr == owner_, "");
          ENGINE_ASSERT(0 == count_, "");
          ENGINE_STATIC_ASSERT(wait_queue_.empty(), "");
        }

        recursive_timed_mutex(recursive_timed_mutex const &) = delete;
        recursive_timed_mutex &operator=(recursive_timed_mutex const &) = delete;

        void lock();

        bool try_lock() noexcept;

        template <typename Clock, typename Duration>
        bool try_lock_until(std::chrono::time_point<Clock, Duration> const &timeout_time_) {
          std::chrono::steady_clock::time_point timeout_time = detail::convert(timeout_time_);
          return try_lock_until_(timeout_time);
        }

        template <typename Rep, typename Period>
        bool try_lock_for(std::chrono::duration<Rep, Period> const &timeout_duration) {
          return try_lock_until_(std::chrono::steady_clock::now() + timeout_duration);
        }

        void unlock();
    };
  }
}

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(pop)
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
