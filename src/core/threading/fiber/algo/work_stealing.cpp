#include "work_stealing.hpp"
#include <random>
#include <core/assert.hpp>
#include <core/threading/context/detail/prefetch.hpp>
#include <core/threading/fiber/detail/thread_barrier.hpp>
#include <core/threading/fiber/type.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace algo {
      std::atomic<std::uint32_t> work_stealing::counter_{0};
      std::vector<RefCountedPtr<work_stealing>> work_stealing::schedulers_{};

      void work_stealing::init_(std::uint32_t thread_count, std::vector<RefCountedPtr<work_stealing>> &schedulers) {
        // resize array of schedulers to thread_count, initilized with nullptr
        std::vector<RefCountedPtr<work_stealing>>{thread_count, nullptr}.swap(schedulers);
      }

      work_stealing::work_stealing(std::uint32_t thread_count, bool suspend):
        id_{counter_++},
        thread_count_{thread_count},
        suspend_{suspend} {
        static Core::Fibers::detail::thread_barrier b{thread_count};
        // initialize the array of schedulers
        static std::once_flag flag;
        std::call_once(flag, &work_stealing::init_, thread_count_, std::ref(schedulers_));
        // register pointer of this scheduler
        schedulers_[id_] = this;
        b.wait();
      }

      void work_stealing::awakened(context *ctx) noexcept {
        if (!ctx->is_context(type::pinned_context)) {
          ctx->detach();
        }
        rqueue_.push(ctx);
      }

      context *work_stealing::pick_next() noexcept {
        context *victim = rqueue_.pop();
        if (nullptr != victim) {
          Core::Context::detail::prefetch_range(victim, sizeof(context));
          if (!victim->is_context(type::pinned_context)) {
            context::active()->attach(victim);
          }
        }
        else {
          std::uint32_t id = 0;
          std::size_t count = 0, size = schedulers_.size();
          static thread_local std::minstd_rand generator{std::random_device{}()};
          std::uniform_int_distribution<std::uint32_t> distribution{
            0, static_cast<std::uint32_t>(thread_count_ - 1)
          };
          do {
            do {
              ++count;
              // random selection of one logical cpu
              // that belongs to the local NUMA node
              id = distribution(generator);
              // prevent stealing from own scheduler
            }
            while (id == id_);
            // steal context from other scheduler
            victim = schedulers_[id]->steal();
          }
          while (nullptr == victim && count < size);
          if (nullptr != victim) {
            Core::Context::detail::prefetch_range(victim, sizeof(context));
            ENGINE_ASSERT(!victim->is_context(type::pinned_context), "");
            context::active()->attach(victim);
          }
        }
        return victim;
      }

      void work_stealing::suspend_until(std::chrono::steady_clock::time_point const &time_point) noexcept {
        if (suspend_) {
          if ((std::chrono::steady_clock::time_point::max)() == time_point) {
            std::unique_lock<std::mutex> lk{mtx_};
            cnd_.wait(lk, [this]() { return flag_; });
            flag_ = false;
          }
          else {
            std::unique_lock<std::mutex> lk{mtx_};
            cnd_.wait_until(lk, time_point, [this]() { return flag_; });
            flag_ = false;
          }
        }
      }

      void work_stealing::notify() noexcept {
        if (suspend_) {
          std::unique_lock<std::mutex> lk{mtx_};
          flag_ = true;
          lk.unlock();
          cnd_.notify_all();
        }
      }
    }
  }
}
//
// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
