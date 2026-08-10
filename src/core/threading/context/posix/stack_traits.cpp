#include "../stack_traits.hpp"

extern "C" {
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
}

//#if _POSIX_C_SOURCE >= 200112L

#include <algorithm>
#include <cmath>
#include <core/assert.hpp>

#if !defined (SIGSTKSZ)
# define SIGSTKSZ (32768) // 32kb minimum allowable stack
# define UDEF_SIGSTKSZ
#endif

#if !defined (MINSIGSTKSZ)
# define MINSIGSTKSZ (131072) // 128kb recommended stack size
# define UDEF_MINSIGSTKSZ
#endif

namespace {
  std::size_t pagesize() noexcept {
    // conform to POSIX.1-2001
    return static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
  }

  rlim_t stacksize_limit_() noexcept {
    rlimit limit;
    // conforming to POSIX.1-2001
    ::getrlimit(RLIMIT_STACK, &limit);
    return limit.rlim_max;
  }

  rlim_t stacksize_limit() noexcept {
    static rlim_t limit = stacksize_limit_();
    return limit;
  }
}

namespace Core::Context {
  bool stack_traits::is_unbounded() noexcept {
    return RLIM_INFINITY == stacksize_limit();
  }

  std::size_t stack_traits::page_size() noexcept {
    static std::size_t size = pagesize();
    return size;
  }

  std::size_t stack_traits::default_size() noexcept {
    return 128 * 1024;
  }

  std::size_t stack_traits::minimum_size() noexcept {
    return static_cast<std::size_t>(MINSIGSTKSZ);
  }

  std::size_t stack_traits::maximum_size() noexcept {
    ENGINE_ASSERT(!is_unbounded(), "");
    return static_cast<std::size_t>(stacksize_limit());
  }
}


#ifdef UDEF_SIGSTKSZ
# undef SIGSTKSZ
#endif

#ifdef UDEF_MINSIGSTKSZ
# undef MINSIGSTKSZ
#endif
