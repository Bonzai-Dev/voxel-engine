#pragma once

#include <chrono>

// #include <boost/config.hpp>

#include <core/threading/fiber/algo/algorithm.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/convert.hpp>
#include <core/threading/fiber/fiber.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include <core/threading/fiber/stack_allocator_wrapper.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace this_fiber {
    inline Fibers::fiber::id get_id() noexcept {
      return Fibers::context::active()->get_id();
    }

    inline
    void yield() noexcept {
      Fibers::context::active()->yield();
    }

    template <typename Clock, typename Duration>
    void sleep_until(std::chrono::time_point<Clock, Duration> const &sleep_time_) {
      std::chrono::steady_clock::time_point sleep_time = Core::Fibers::detail::convert(sleep_time_);
      Fibers::context *active_ctx = Fibers::context::active();
      active_ctx->wait_until(sleep_time);
    }

    template <typename Rep, typename Period>
    void sleep_for(std::chrono::duration<Rep, Period> const &timeout_duration) {
      Fibers::context *active_ctx = Fibers::context::active();
      active_ctx->wait_until(std::chrono::steady_clock::now() + timeout_duration);
    }

    template <typename PROPS>
    PROPS &properties() {
      Fibers::fiber_properties *props = Fibers::context::active()->get_properties();
      if (nullptr == props) [[likely]] {
        // props could be nullptr if the thread's main fiber has not yet
        // yielded (not yet passed through algorithm_with_properties::
        // awakened()). Address that by yielding right now.
        yield();
        // Try again to obtain the fiber_properties subclass instance ptr.
        // Walk through the whole chain again because who knows WHAT might
        // have happened while we were yielding!
        props = Fibers::context::active()->get_properties();
        // Could still be hosed if the running manager isn't a subclass of
        // algorithm_with_properties.
        ENGINE_ASSERT(props, "this_fiber::properties not set");
      }
      return dynamic_cast<PROPS&>(*props);
    }
  }

  namespace Fibers {
    inline
    bool has_ready_fibers() noexcept {
      return Core::Fibers::context::active()->get_scheduler()->has_ready_fibers();
    }

    // Returns true if the thread could be initialize, false otherwise (it was already initialized previously).
    inline bool initialize_thread(algo::algorithm::ptr_t algo, stack_allocator_wrapper &&salloc) noexcept {
      return Core::Fibers::context::initialize_thread(algo, std::move(salloc));
    }

    template <typename SchedAlgo, typename... Args>
    void use_scheduling_algorithm(Args &&... args) noexcept {
      initialize_thread(new SchedAlgo(std::forward<Args>(args)...),
                        make_stack_allocator_wrapper<Core::Fibers::default_stack>());
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
