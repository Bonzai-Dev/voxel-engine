#include "round_robin.hpp"
#include <core/assert.hpp>
#include <core/threading/context/detail/prefetch.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace algo {
      void round_robin::awakened(context *ctx) noexcept {
        ENGINE_ASSERT(nullptr != ctx, "");
        ENGINE_ASSERT(! ctx->ready_is_linked(), "");
        ENGINE_ASSERT(ctx->is_resumable(), "");
        ctx->ready_link(rqueue_);
      }

      context *round_robin::pick_next() noexcept {
        context *victim = nullptr;
        if (!rqueue_.empty()) {
          victim = &rqueue_.front();
          rqueue_.pop_front();
          Core::Context::detail::prefetch_range(victim, sizeof(context));
          ENGINE_ASSERT(nullptr != victim, "");
          ENGINE_ASSERT(!victim->ready_is_linked(), "");
          ENGINE_ASSERT(victim->is_resumable(), "");
        }
        return victim;
      }

      bool round_robin::has_ready_fibers() const noexcept {
        return !rqueue_.empty();
      }

      void round_robin::suspend_until(std::chrono::steady_clock::time_point const &time_point) noexcept {
        if ((std::chrono::steady_clock::time_point::max)() == time_point) {
          std::unique_lock<std::mutex> lk{mtx_};
          cnd_.wait(lk, [&]() { return flag_; });
          flag_ = false;
        }
        else {
          std::unique_lock<std::mutex> lk{mtx_};
          cnd_.wait_until(lk, time_point, [&]() { return flag_; });
          flag_ = false;
        }
      }

      void round_robin::notify() noexcept {
        std::unique_lock<std::mutex> lk{mtx_};
        flag_ = true;
        lk.unlock();
        cnd_.notify_all();
      }
    }
  }
}
//
// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
