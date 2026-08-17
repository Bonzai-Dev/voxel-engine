#pragma once
#include <cstdint>
#include <thread>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
      void pin_thread(std::uint32_t, std::thread::native_handle_type);

      void pin_thread(std::uint32_t cpuid);
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
