#pragma once

// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include "spinlock_ttas.hpp"

#if !defined(BOOST_FIBERS_NO_ATOMICS)
# include <mutex>
# include <core/threading/fiber/detail/spinlock_ttas_adaptive.hpp>
# include <core/threading/fiber/detail/spinlock_ttas.hpp>
# if defined(BOOST_FIBERS_HAS_FUTEX)
#  include <boost/fiber/detail/spinlock_ttas_adaptive_futex.hpp>
#  include <boost/fiber/detail/spinlock_ttas_futex.hpp>
# endif
# if defined(BOOST_USE_TSX)
#  include <boost/fiber/detail/spinlock_rtm.hpp>
# endif
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace Core {
  namespace Fibers {
    namespace detail {
#if defined(BOOST_FIBERS_NO_ATOMICS)
      struct spinlock {
        constexpr spinlock() noexcept {
        }

        void lock() noexcept {
        }

        void unlock() noexcept {
        }
      };

      struct spinlock_lock {
        constexpr spinlock_lock(spinlock &) noexcept {
        }

        void lock() noexcept {
        }

        void unlock() noexcept {
        }
      };
#else
# if defined(BOOST_FIBERS_SPINLOCK_STD_MUTEX)
      using spinlock = std::mutex;
# elif defined(BOOST_FIBERS_SPINLOCK_TTAS_FUTEX)
#  if defined(BOOST_USE_TSX)
      using spinlock = spinlock_rtm<spinlock_ttas_futex>;
#  else
      using spinlock = spinlock_ttas_futex;
#  endif
# elif defined(BOOST_FIBERS_SPINLOCK_TTAS_ADAPTIVE_FUTEX)
#  if defined(BOOST_USE_TSX)
      using spinlock = spinlock_rtm<spinlock_ttas_adaptive_futex>;
#  else
      using spinlock = spinlock_ttas_adaptive_futex;
#  endif
# elif defined(BOOST_FIBERS_SPINLOCK_TTAS_ADAPTIVE)
#  if defined(BOOST_USE_TSX)
      using spinlock = spinlock_rtm<spinlock_ttas_adaptive>;
#  else
      using spinlock = spinlock_ttas_adaptive;
#  endif
# else
#  if defined(BOOST_USE_TSX)
      using spinlock = spinlock_rtm<spinlock_ttas>;
#  else
      using spinlock = spinlock_ttas;
#  endif
# endif
      using spinlock_lock = std::unique_lock<spinlock>;
#endif
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
