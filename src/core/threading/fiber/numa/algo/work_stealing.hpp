#pragma once
#include <condition_variable>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

// #include <boost/config.hpp>
#include <core/memory.hpp>

#include <core/threading/fiber/algo/algorithm.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/context_spinlock_queue.hpp>
#include <core/threading/fiber/detail/context_spmc_queue.hpp>
#include <core/threading/fiber/numa/pin_thread.hpp>
#include <core/threading/fiber/numa/topology.hpp>
#include <core/threading/fiber/scheduler.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace numa {
      namespace algo {
        class work_stealing: public Core::Fibers::algo::algorithm {
          private:
            static std::vector<RefCountedPtr<work_stealing>> schedulers_;

            std::uint32_t cpu_id_;
            std::vector<std::uint32_t> local_cpus_;
            std::vector<std::uint32_t> remote_cpus_;
#ifdef BOOST_FIBERS_USE_SPMC_QUEUE
            detail::context_spmc_queue rqueue_{};
#else
            detail::context_spinlock_queue rqueue_{};
#endif
            std::mutex mtx_{};
            std::condition_variable cnd_{};
            bool flag_{false};
            bool suspend_;

            static void init_(
              std::vector<Core::Fibers::numa::node> const &,
              std::vector<RefCountedPtr<work_stealing>> &
            );

          public:
            work_stealing(
              std::uint32_t,
              std::uint32_t,
              std::vector<Core::Fibers::numa::node> const &,
              bool = false
            );


            work_stealing(work_stealing const &) = delete;
            work_stealing(work_stealing &&) = delete;

            work_stealing &operator=(work_stealing const &) = delete;
            work_stealing &operator=(work_stealing &&) = delete;

            virtual void awakened(context *) noexcept;

            virtual context *pick_next() noexcept;

            virtual context *steal() noexcept {
              return rqueue_.steal();
            }

            virtual bool has_ready_fibers() const noexcept {
              return !rqueue_.empty();
            }

            virtual void suspend_until(std::chrono::steady_clock::time_point const &) noexcept;

            virtual void notify() noexcept;
        };
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
