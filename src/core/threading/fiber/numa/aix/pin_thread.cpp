#include <core/threading/fiber/numa/pin_thread.hpp>

extern "C" {
#include <sys/errno.h>
#include <sys/processor.h>
#include <sys/thread.h>
}

#include <system_error>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
      void pin_thread(std::uint32_t cpuid) {
        pin_thread(cpuid, ::thread_self());
      }


      void pin_thread(std::uint32_t cpuid, std::thread::native_handle_type h) {
        if (-1 == ::bindprocessor(BINDTHREAD, h, static_cast<cpu_t>(cpuid))) [[unlikely]] {
          throw std::system_error(
            std::error_code(errno, std::system_category()),
            "bindprocessor() failed");
        }
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
