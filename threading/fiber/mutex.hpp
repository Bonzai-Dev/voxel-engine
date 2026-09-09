//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
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

namespace boost {
  namespace fibers {
    class condition_variable;

    class mutex {
      private:
        friend class condition_variable;

        detail::spinlock wait_queue_splk_{};
        wait_queue wait_queue_{};
        context *owner_{nullptr};

      public:
        mutex() = default;

        ~mutex() {
          ENGINE_ASSERT(nullptr == owner_, "");
          ENGINE_ASSERT(wait_queue_.empty(), "");
        }

        mutex(mutex const &) = delete;
        mutex &operator=(mutex const &) = delete;

        void lock();

        bool try_lock();

        void unlock();
    };
  }
}

#ifdef ENGINE_COMPILER_MSVC
# pragma warning(pop)
#endif
