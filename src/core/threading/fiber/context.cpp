#include <core/threading/fiber/context.hpp>
#include <cstdlib>
#include <mutex>
#include <new>
#include <core/threading/fiber/exceptions.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include <core/threading/fiber/algo/round_robin.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    class main_context final: public context {
      public:
        main_context() noexcept:
          context{1, type::main_context, launch::post} {
        }
    };

    class dispatcher_context final: public context {
      private:
        Core::Context::fiber
        run_(Core::Context::fiber &&) {
          // execute scheduler::dispatch()
          return get_scheduler()->dispatch();
        }

      public:
        dispatcher_context(Core::Context::preallocated const &palloc, stack_allocator_wrapper &&salloc):
          context{0, type::dispatcher_context, launch::post} {
          c_ = Core::Context::fiber{
            std::allocator_arg, palloc, std::move(salloc),
            std::bind(&dispatcher_context::run_, this, std::placeholders::_1)
          };
        }
    };

    static RefCountedPtr<context> make_dispatcher_context(stack_allocator_wrapper &&salloc) {
      auto sctx = salloc.allocate();
      // reserve space for control structure
      void *storage = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sizeof(dispatcher_context)))
        & ~static_cast<uintptr_t>(0xff));
      void *stack_bottom = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
      const std::size_t size = reinterpret_cast<uintptr_t>(storage) - reinterpret_cast<uintptr_t>(stack_bottom);
      // placement new of context on top of fiber's stack
      return RefCountedPtr<context>{
        new(storage) dispatcher_context{
          Core::Context::preallocated{storage, size, sctx}, std::move(salloc)
        }
      };
    }

    // schwarz counter
    struct context_initializer {
      static thread_local context *active_;
      static thread_local std::size_t counter_;

      using default_scheduler = algo::round_robin;

      template <typename... Args>
      context_initializer(Args &&... args) {
        if (0 == counter_++) {
          initialize(std::forward<Args>(args)...);
        }
      }

      ~context_initializer() {
        if (0 == --counter_) {
          deinitialize();
        }
      }

      void initialize() {
        initialize(new default_scheduler(), make_stack_allocator_wrapper<default_stack>());
      }

      void initialize(algo::algorithm::ptr_t algo, stack_allocator_wrapper &&salloc) {
        // main fiber context of this thread
        context *main_ctx = new main_context{};
        // scheduler of this thread
        auto sched = new scheduler(algo);
        // attach main context to scheduler
        sched->attach_main_context(main_ctx);
        // create and attach dispatcher context to scheduler
        sched->attach_dispatcher_context(make_dispatcher_context(std::move(salloc)));
        // make main context to active context
        active_ = main_ctx;
      }

      void deinitialize() {
        context *main_ctx = active_;
        ENGINE_ASSERT(main_ctx->is_context(type::main_context), "");
        scheduler *sched = main_ctx->get_scheduler();
        delete sched;
        delete main_ctx;
      }
    };

    // zero-initialization
    thread_local context *context_initializer::active_{nullptr};
    thread_local std::size_t context_initializer::counter_{0};

    bool context::initialize_thread(algo::algorithm::ptr_t algo, stack_allocator_wrapper &&salloc) noexcept {
      if (context_initializer::counter_ == 0) {
        // Initilization is not done yet, so do it now with a local variable
        // context_initializer which will decrease the counter when leaving this function.
        context_initializer ctx_initializer(algo, std::move(salloc));

        // Now call active() to register a thread local context_initializer which will
        // ensure resources are free'ed when the thread exits.
        active();

        return true;
      }
      else {
        // It's too late already to initialize the dispatcher stack allocator, still we can update
        // the algo.
        active()->get_scheduler()->set_algo(algo);

        return false;
      }
    }

    context *context::active() noexcept {
      // initialized the first time control passes; per thread
      thread_local static context_initializer ctx_initializer;
      return context_initializer::active_;
    }

    void context::reset_active() noexcept {
      context_initializer::active_ = nullptr;
    }

    context::~context() {
      // protect for concurrent access
      std::unique_lock<detail::spinlock> lk{splk_};
      ENGINE_ASSERT(!ready_is_linked(), "");
      ENGINE_ASSERT(!remote_ready_is_linked(), "");
      ENGINE_ASSERT(!sleep_is_linked(), "");
      if (is_context(type::dispatcher_context)) {
        ENGINE_ASSERT(nullptr == active(), "");
      }
      ENGINE_ASSERT(wait_queue_.empty(), "");
      delete properties_;
    }

    context::id context::get_id() const noexcept {
      return id{const_cast<context*>(this)};
    }

    void context::resume() noexcept {
      context *prev = this;
      // context_initializer::active_ will point to `this`
      // prev will point to previous active context
      std::swap(context_initializer::active_, prev);
      // pass pointer to the context that resumes `this`
      std::move(c_).resume_with([prev](Core::Context::fiber &&c) {
        prev->c_ = std::move(c);
        return Core::Context::fiber{};
      });
    }

    void context::resume(detail::spinlock_lock &lk) noexcept {
      context *prev = this;
      // context_initializer::active_ will point to `this`
      // prev will point to previous active context
      std::swap(context_initializer::active_, prev);
      // pass pointer to the context that resumes `this`
      std::move(c_).resume_with([prev,&lk](Core::Context::fiber &&c) {
        prev->c_ = std::move(c);
        lk.unlock();
        return Core::Context::fiber{};
      });
    }

    void context::resume(context *ready_ctx) noexcept {
      context *prev = this;
      // context_initializer::active_ will point to `this`
      // prev will point to previous active context
      std::swap(context_initializer::active_, prev);
      // pass pointer to the context that resumes `this`
      std::move(c_).resume_with([prev,ready_ctx](Core::Context::fiber &&c) {
        prev->c_ = std::move(c);
        context::active()->schedule(ready_ctx);
        return Core::Context::fiber{};
      });
    }

    void context::suspend() noexcept {
      get_scheduler()->suspend();
    }

    void context::suspend(detail::spinlock_lock &lk) noexcept {
      get_scheduler()->suspend(lk);
    }

    void context::join() {
      // get active context
      context *active_ctx = context::active();
      // protect for concurrent access
      std::unique_lock<detail::spinlock> lk{splk_};
      // wait for context which is not terminated
      if (!terminated_) {
        // push active context to wait-queue, member
        // of the context which has to be joined by
        // the active context
        wait_queue_.suspend_and_wait(lk, active_ctx);
        // active context resumed
        ENGINE_ASSERT(context::active() == active_ctx, "");
      }
    }

    void context::yield() noexcept {
      // yield active context
      get_scheduler()->yield(context::active());
    }

    Core::Context::fiber context::suspend_with_cc() noexcept {
      context *prev = this;
      // context_initializer::active_ will point to `this`
      // prev will point to previous active context
      std::swap(context_initializer::active_, prev);
      // pass pointer to the context that resumes `this`
      return std::move(c_).resume_with([prev](Core::Context::fiber &&c) {
        prev->c_ = std::move(c);
        return Core::Context::fiber{};
      });
    }

    Core::Context::fiber context::terminate() noexcept {
      // protect for concurrent access
      std::unique_lock<detail::spinlock> lk{splk_};
      // mark as terminated
      terminated_ = true;
      // notify all waiting fibers
      wait_queue_.notify_all();
      ENGINE_ASSERT(wait_queue_.empty(), "");
      // release fiber-specific-data
      for (fss_data_t::value_type &data : fss_data_) {
        data.second.do_cleanup();
      }
      fss_data_.clear();
      // switch to another context
      return get_scheduler()->terminate(lk, this);
    }

    bool context::wait_until(std::chrono::steady_clock::time_point const &tp) noexcept {
      ENGINE_ASSERT(nullptr != get_scheduler(), "");
      ENGINE_ASSERT(this == active(), "");
      return get_scheduler()->wait_until(this, tp);
    }

    bool context::wait_until(std::chrono::steady_clock::time_point const &tp,
                             detail::spinlock_lock &lk,
                             waker &&w) noexcept {
      ENGINE_ASSERT(nullptr != get_scheduler(), "");
      ENGINE_ASSERT(this == active(), "");
      return get_scheduler()->wait_until(this, tp, lk, std::move(w));
    }


    bool context::wake(const size_t epoch) noexcept {
      size_t expected = epoch;
      bool is_last_waker = waker_epoch_.compare_exchange_strong(expected, epoch + 1, std::memory_order_acq_rel);
      if (!is_last_waker) {
        // waker_epoch_ has been incremented before, so consider this wake
        // operation as outdated and do nothing
        return false;
      }

      ENGINE_ASSERT(context::active() != this, "");
      if (context::active()->get_scheduler() == get_scheduler()) {
        get_scheduler()->schedule(this);
      }
      else {
        get_scheduler()->schedule_from_remote(this);
      }
      return true;
    }

    void context::schedule(context *ctx) noexcept {
      //BOOST_ASSERT( nullptr != ctx);
      ENGINE_ASSERT(this != ctx, "");
      ENGINE_ASSERT(nullptr != get_scheduler(), "");
      ENGINE_ASSERT(nullptr != ctx->get_scheduler(), "");

      // FIXME: comparing scheduler address' must be synchronized?
      //        what if ctx is migrated between threads
      //        (other scheduler assigned)
      if (scheduler_ == ctx->get_scheduler()) {
        // local
        get_scheduler()->schedule(ctx);
      }
      else {
        // remote
        ctx->get_scheduler()->schedule_from_remote(ctx);
      }
    }

    void *context::get_fss_data(void const *vp) const {
      auto key = reinterpret_cast<uintptr_t>(vp);
      auto i = fss_data_.find(key);
      return fss_data_.end() != i ? i->second.vp : nullptr;
    }

    void context::set_fss_data(void const *vp,
                               detail::fss_cleanup_function::ptr_t const &cleanup_fn,
                               void *data,
                               bool cleanup_existing) {
      ENGINE_ASSERT(cleanup_fn, "");
      auto key = reinterpret_cast<uintptr_t>(vp);
      auto i = fss_data_.find(key);
      if (fss_data_.end() != i) {
        if (cleanup_existing) {
          i->second.do_cleanup();
        }
        if (nullptr != data) {
          i->second = fss_data{data, cleanup_fn};
        }
        else {
          fss_data_.erase(i);
        }
      }
      else {
        fss_data_.insert(std::make_pair(key, fss_data{data, cleanup_fn}));
      }
    }

    void context::set_properties(fiber_properties *props) noexcept {
      delete properties_;
      properties_ = props;
    }

    bool context::worker_is_linked() const noexcept {
      return worker_hook_.is_linked();
    }

    bool context::ready_is_linked() const noexcept {
      return ready_hook_.is_linked();
    }

    bool context::remote_ready_is_linked() const noexcept {
      return remote_ready_hook_.is_linked();
    }

    bool context::sleep_is_linked() const noexcept {
      return sleep_hook_.is_linked();
    }

    bool context::terminated_is_linked() const noexcept {
      return terminated_hook_.is_linked();
    }

    void context::worker_unlink() noexcept {
      ENGINE_ASSERT(worker_is_linked(), "");
      worker_hook_.unlink();
    }

    void context::ready_unlink() noexcept {
      ENGINE_ASSERT(ready_is_linked(), "");
      ready_hook_.unlink();
    }

    void context::sleep_unlink() noexcept {
      ENGINE_ASSERT(sleep_is_linked(), "");
      sleep_hook_.unlink();
    }

    void context::detach() noexcept {
      ENGINE_ASSERT(context::active() != this, "");
      get_scheduler()->detach_worker_context(this);
    }

    void context::attach(context *ctx) noexcept {
      ENGINE_ASSERT(nullptr != ctx, "");
      get_scheduler()->attach_worker_context(ctx);
    }
  }
}

namespace std {
  std::size_t hash<::Core::Fibers::context::id>::operator()(::Core::Fibers::context::id const &id) const noexcept {
    return reinterpret_cast<std::size_t>(id.impl_);
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
