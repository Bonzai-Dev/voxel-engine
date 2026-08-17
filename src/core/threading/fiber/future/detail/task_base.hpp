#pragma once

// #include <boost/config.hpp>
#include <core/memory.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/future/detail/shared_state.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename R, typename... Args>
      struct task_base: public shared_state<R> {
        typedef RefCountedPtr<task_base> ptr_type;

        virtual ~task_base() {
        }

        virtual void run(Args &&... args) = 0;

        virtual ptr_type reset() = 0;
      };
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
