#pragma once

#include <cstdint>

#include <core/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/detail/config.hpp>

#include "core/application.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      struct rtm_status {
        enum {
          none = 0,
          explicit_abort = 1 << 0,
          may_retry = 1 << 1,
          memory_conflict = 1 << 2,
          buffer_overflow = 1 << 3,
          debug_hit = 1 << 4,
          nested_abort = 1 << 5
        };

        static constexpr std::uint32_t success = ~std::uint32_t{0};
      };

      static ENGINE_FORCE_INLINE std::uint32_t rtm_begin() noexcept {
        std::uint32_t result = rtm_status::success;
        __asm__ __volatile__
        (
          ".byte 0xc7,0xf8 ; .long 0"
          : "+a" (result)
          :
          : "memory"
        );
        return result;
      }

      static ENGINE_FORCE_INLINE void rtm_end() noexcept {
        __asm__ __volatile__
        (
          ".byte 0x0f,0x01,0xd5"
          :
          :
          : "memory"
        );
      }

      static ENGINE_FORCE_INLINE void rtm_abort_lock_not_free() noexcept {
        __asm__ __volatile__
        (
          ".byte 0xc6,0xf8,0xff"
          :
          :
          : "memory"
        );
      }

      static ENGINE_FORCE_INLINE bool rtm_test() noexcept {
        bool result;
        __asm__ __volatile__
        (
          ".byte 0x0f,0x01,0xd6; setz %0"
          : "=q" (result)
          :
          : "memory"
        );
        return result;
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
