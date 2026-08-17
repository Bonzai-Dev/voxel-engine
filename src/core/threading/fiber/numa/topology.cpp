#include <core/threading/fiber/numa/topology.hpp>
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
#pragma message "topology() not supported"
#endif

      std::vector<node> topology() {
        throw fiber_error{
          std::make_error_code(std::errc::function_not_supported),
          "boost fiber: topology() not supported"
        };
        return std::vector<node>{};
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// # include BOOST_ABI_SUFFIX
// #endif
