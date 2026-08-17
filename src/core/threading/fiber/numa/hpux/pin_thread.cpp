#include <core/threading/fiber/numa/pin_thread.hpp>

extern "C" {
#include <sys/pthread.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace Core {
  namespace Fibers {
    namespace numa {
      void pin_thread(std::uint32_t cpuid) {
        pin_thread(cpuid, PTHREAD_SELFTID_NP);
      }


      void pin_thread(std::uint32_t cpuid, std::thread::native_handle_type h) {
        pthread_spu_t spu;
        int err = ::pthread_processor_bind_np(PTHREAD_BIND_FORCED_NP, &spu, static_cast<pthread_spu_t>(cpuid), h);
        if (0 != err) [[unlikely]]
          throw std::system_error(
            std::error_code(err, std::system_category()),
            "pthread_processor_bind_np() failed");
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
