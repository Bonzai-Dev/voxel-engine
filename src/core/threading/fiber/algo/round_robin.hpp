#pragma once
#include <condition_variable>
#include <chrono>
#include <mutex>
// #include <boost/config.hpp>
#include <core/threading/fiber/algo/algorithm.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include "algorithm.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace Core {
  namespace Fibers {
    namespace algo {
      class round_robin: public algorithm {
        private:
          typedef scheduler::ready_queue_type rqueue_type;

          rqueue_type rqueue_{};
          std::mutex mtx_{};
          std::condition_variable cnd_{};
          bool flag_{false};

        public:
          round_robin() = default;

          round_robin(round_robin const &) = delete;
          round_robin &operator=(round_robin const &) = delete;

          void awakened(context *) noexcept override;

          context *pick_next() noexcept override;

          bool has_ready_fibers() const noexcept override;

          void suspend_until(std::chrono::steady_clock::time_point const &) noexcept override;

          void notify() noexcept override;
      };
    }
  }
}

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(pop)
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
