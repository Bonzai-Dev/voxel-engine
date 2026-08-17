#pragma once

#include <atomic>
#include <cstddef>

// #include <boost/config.hpp>
#include <core/memory.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      class fss_cleanup_function {
        private:
          std::atomic<std::size_t> use_count_{0};

        public:
          typedef RefCountedPtr<fss_cleanup_function> ptr_t;

          fss_cleanup_function() = default;

          virtual ~fss_cleanup_function() = default;

          virtual void operator()(void *data) = 0;

          friend inline void intrusive_ptr_add_ref(fss_cleanup_function *p) noexcept {
            p->use_count_.fetch_add(1, std::memory_order_relaxed);
          }

          friend inline void intrusive_ptr_release(fss_cleanup_function *p) noexcept {
            if (1 == p->use_count_.fetch_sub(1, std::memory_order_release)) {
              std::atomic_thread_fence(std::memory_order_acquire);
              delete p;
            }
          }
      };
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
