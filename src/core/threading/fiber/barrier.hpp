#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/threading/fiber/condition_variable.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/mutex.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    class barrier {
      private:
        std::size_t initial_;
        std::size_t current_;
        std::size_t cycle_{0};
        mutex mtx_{};
        condition_variable cond_{};

      public:
        explicit barrier(std::size_t);

        barrier(barrier const &) = delete;
        barrier &operator=(barrier const &) = delete;

        bool wait();
    };
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
