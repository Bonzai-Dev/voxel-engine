//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "topology.hpp"
#include <system_error>
#include "../exceptions.hpp"

namespace boost::fibers::numa {
#if ENGINE_CORE_CLANG || \
    ENGINE_COMPILER_GCC || \
    BOOST_COMP_INTEL ||  \
    ENGINE_COMPILER_MSVC
# pragma message "topology() not supported"
#endif

  std::vector<node> topology() {
    throw fiber_error{
      std::make_error_code(std::errc::function_not_supported),
      "boost fiber: topology() not supported"
    };
    return std::vector<node>{};
  }
}
