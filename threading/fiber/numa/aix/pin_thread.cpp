//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "../pin_thread.hpp"

extern "C" {
#include <sys/errno.h>
#include <sys/processor.h>
#include <sys/thread.h>
}

#include <system_error>

namespace boost {
  namespace fibers {
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
