#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

// #include <boost/config.hpp>
#include <core/threading/context/fiber.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/slist.hpp>

#include <core/threading/fiber/algo/algorithm.hpp>
#include <core/threading/fiber/context.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/data.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

#ifdef ENGINE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace Core {
  namespace Fibers {
    class scheduler {
      public:
        struct timepoint_less {
          bool operator()(context const &l, context const &r) const noexcept {
            return l.tp_ < r.tp_;
          }
        };

        typedef intrusive::list<
          context,
          intrusive::member_hook<
            context, detail::ready_hook, &context::ready_hook_>,
          intrusive::constant_time_size<false>
        > ready_queue_type;

      private:
        typedef intrusive::multiset<
          context,
          intrusive::member_hook<
            context, detail::sleep_hook, &context::sleep_hook_>,
          intrusive::constant_time_size<false>,
          intrusive::compare<timepoint_less>
        > sleep_queue_type;
        typedef intrusive::list<
          context,
          intrusive::member_hook<
            context, detail::worker_hook, &context::worker_hook_>,
          intrusive::constant_time_size<false>
        > worker_queue_type;
        typedef intrusive::slist<
          context,
          intrusive::member_hook<
            context, detail::terminated_hook, &context::terminated_hook_>,
          intrusive::linear<true>,
          intrusive::cache_last<true>
        > terminated_queue_type;
        typedef intrusive::slist<
          context,
          intrusive::member_hook<
            context, detail::remote_ready_hook, &context::remote_ready_hook_>,
          intrusive::linear<true>,
          intrusive::cache_last<true>
        > remote_ready_queue_type;

#if ! defined(BOOST_FIBERS_NO_ATOMICS)
        // remote ready-queue contains context' signaled by schedulers
        // running in other threads
        detail::spinlock remote_ready_splk_{};
        remote_ready_queue_type remote_ready_queue_{};
#endif
        algo::algorithm::ptr_t algo_;
        // sleep-queue contains context' which have been called
        // scheduler::wait_until()
        sleep_queue_type sleep_queue_{};
        // worker-queue contains all context' managed by this scheduler
        // except main-context and dispatcher-context
        // unlink happens on destruction of a context
        worker_queue_type worker_queue_{};
        // terminated-queue contains context' which have been terminated
        terminated_queue_type terminated_queue_{};
        RefCountedPtr<context> dispatcher_ctx_{};
        context *main_ctx_{nullptr};
        bool shutdown_{false};

        void release_terminated_() noexcept;

        void remote_ready2ready_() noexcept;

        void sleep2ready_() noexcept;

      public:
        scheduler(algo::algorithm::ptr_t algo) noexcept;

        scheduler(scheduler const &) = delete;
        scheduler &operator=(scheduler const &) = delete;

        virtual ~scheduler();

        void schedule(context *) noexcept;

        void schedule_from_remote(context *) noexcept;

        Core::Context::fiber dispatch() noexcept;

        Core::Context::fiber terminate(detail::spinlock_lock &, context *) noexcept;

        void yield(context *) noexcept;

        bool wait_until(context *, std::chrono::steady_clock::time_point const &) noexcept;

        bool wait_until(context *, std::chrono::steady_clock::time_point const &, detail::spinlock_lock &, waker &&) noexcept;

        void suspend() noexcept;
        void suspend(detail::spinlock_lock &) noexcept;

        bool has_ready_fibers() const noexcept;

        void set_algo(algo::algorithm::ptr_t) noexcept;

        void attach_main_context(context *) noexcept;

        void attach_dispatcher_context(RefCountedPtr<context>) noexcept;

        void attach_worker_context(context *) noexcept;

        void detach_worker_context(context *) noexcept;
    };
  }
}

#ifdef ENGINE_COMPILER_MSVC
# pragma warning(pop)
#endif

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
