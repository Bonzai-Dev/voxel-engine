//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/assert.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>
#include <core/threading/fiber/waker.hpp>

#ifdef ENGINE_COMPILER_MSVC
# pragma warning(push)
# pragma warning(disable:4251)
#endif


namespace boost::fibers {
  class condition_variable;

  class recursive_mutex {
    private:
      friend class condition_variable;

      detail::spinlock wait_queue_splk_{};
      wait_queue wait_queue_{};
      context *owner_{nullptr};
      std::size_t count_{0};

    public:
      recursive_mutex() = default;

      ~recursive_mutex() {
        ENGINE_ASSERT(nullptr == owner_, "");
        ENGINE_ASSERT(0 == count_, "");
        ENGINE_ASSERT(wait_queue_.empty(), "");
      }

      recursive_mutex(recursive_mutex const &) = delete;
      recursive_mutex &operator=(recursive_mutex const &) = delete;

      void lock();

      bool try_lock() noexcept;

      void unlock();
  };
}

#ifdef ENGINE_COMPILER_MSVC
# pragma warning(pop)
#endif
