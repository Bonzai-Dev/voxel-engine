#include <core/threading/fiber/barrier.hpp>

#include <mutex>
#include <system_error>
#include <core/threading/fiber/exceptions.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    barrier::barrier(std::size_t initial):
      initial_{initial},
      current_{initial_} {
      if (0 == initial) [[unlikely]] {
        throw fiber_error{
          std::make_error_code(std::errc::invalid_argument),
          "boost fiber: zero initial barrier count"
        };
      }
    }

    bool barrier::wait() {
      std::unique_lock<mutex> lk{mtx_};
      const std::size_t cycle = cycle_;
      if (0 == --current_) {
        ++cycle_;
        current_ = initial_;
        lk.unlock(); // no pessimization
        cond_.notify_all();
        return true;
      }

      cond_.wait(lk, [&] { return cycle != cycle_; });
      return false;
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
