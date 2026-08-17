#include <core/threading/fiber/numa/pin_thread.hpp>
#include <system_error>
#include <core/threading/fiber/exceptions.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
#if ENGINE_COMPILER_CLANG || \
ENGINE_COMPILER_GCC || \
    BOOST_COMP_INTEL ||  \
    ENGINE_COMPILER_MSVC
# pragma message "pin_thread() not supported"
#endif

      void pin_thread(std::uint32_t) {
        throw fiber_error{
          std::make_error_code(std::errc::function_not_supported),
          "boost fiber: pin_thread() not supported"
        };
      }

      void pin_thread(std::uint32_t cpuid, std::thread::native_handle_type h) {
        throw fiber_error{
          std::make_error_code(std::errc::function_not_supported),
          "boost fiber: pin_thread() not supported"
        };
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
