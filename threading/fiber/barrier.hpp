//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/threading/fiber/condition_variable.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/mutex.hpp>

namespace boost {
  namespace fibers {
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
