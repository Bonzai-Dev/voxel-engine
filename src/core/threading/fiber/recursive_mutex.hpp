#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/assert.hpp>
#include "context.hpp"
#include "detail/config.hpp"
#include "detail/spinlock.hpp"
#include "waker.hpp"

// // #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace Core {
  namespace Fibers {
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
}

#ifdef ENGINE_COMPILER_MSVC
# pragma warning(pop)
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
