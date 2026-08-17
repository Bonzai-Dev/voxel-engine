#pragma once

#include <cstddef>
#include <condition_variable>
#include <mutex>

#include <core/assert.hpp>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      class thread_barrier {
        private:
          std::size_t initial_;
          std::size_t current_;
          bool cycle_{true};
          std::mutex mtx_{};
          std::condition_variable cond_{};

        public:
          explicit thread_barrier(std::size_t initial):
            initial_{initial},
            current_{initial_} {
            ENGINE_ASSERT(0 != initial, "");
          }

          thread_barrier(thread_barrier const &) = delete;
          thread_barrier &operator=(thread_barrier const &) = delete;

          bool wait() {
            std::unique_lock<std::mutex> lk(mtx_);
            const bool cycle = cycle_;
            if (0 == --current_) {
              cycle_ = !cycle_;
              current_ = initial_;
              lk.unlock(); // no pessimization
              cond_.notify_all();
              return true;
            }
            cond_.wait(lk, [&]() { return cycle != cycle_; });
            return false;
          }
      };
    }
  }
}
