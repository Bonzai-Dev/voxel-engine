#include <core/threading/fiber/numa/pin_thread.hpp>

extern "C" {
#include <pthread.h>
#include <pthread_np.h>
}

#include <system_error>
//
// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
      void pin_thread(std::uint32_t cpuid) {
        pin_thread(cpuid, ::pthread_self());
      }


      void pin_thread(std::uint32_t cpuid, std::thread::native_handle_type h) {
        cpuset_t set;
        CPU_ZERO(& set);
        CPU_SET(cpuid, & set);
        int err = 0;
        if (0 != (err = ::pthread_setaffinity_np(h, sizeof(set), &set))) [[unlikely]] {
          throw std::system_error(
            std::error_code(err, std::system_category()),
            "pthread_setaffinity_np() failed");
        }
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
