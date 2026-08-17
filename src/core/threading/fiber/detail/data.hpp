#pragma once

// #include <boost/config.hpp>

#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    class context;

    namespace detail {
      struct data_t {
        spinlock_lock *lk{nullptr};
        context *ctx{nullptr};
        context *from;

        explicit data_t(context *from_) noexcept:
          from{from_} {
        }

        explicit data_t(spinlock_lock *lk_,
                        context *from_) noexcept:
          lk{lk_},
          from{from_} {
        }

        explicit data_t(context *ctx_,
                        context *from_) noexcept:
          ctx{ctx_},
          from{from_} {
        }
      };
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
