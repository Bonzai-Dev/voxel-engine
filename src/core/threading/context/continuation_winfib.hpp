#pragma once

#include <windows.h>
#include "detail/config.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <ostream>
#include <system_error>
#include <tuple>
#include <utility>
#include <core/assert.hpp>
#include <boost/config.hpp>

#include "continuation_ucontext.hpp"
#include "core/threading/fiber/fixedsize_stack.hpp"
#include "detail/disable_overload.hpp"

#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
#include "detail/exchange.hpp"
#endif
#if defined(BOOST_NO_CXX17_STD_INVOKE)
#include "detail/invoke.hpp"
#endif
#include "fixedsize_stack.hpp"
#include "flags.hpp"
#include "preallocated.hpp"
#include "stack_context.hpp"

#if defined(ENGINE_COMPILER_MSVC)
# pragma warning(push)
# pragma warning(disable: 4702)
#endif

namespace Core::Context {
  // tampoline function
  // entered if the execution context
  // is resumed for the first time
  template<typename Record>
  static VOID WINAPI entry_func(LPVOID data) noexcept {
    Record *record = static_cast<Record *>(data);
    ENGINE_STATIC_ASSERT(nullptr != record);
    // start execution of toplevel context-function
    record->run();
  }

  struct activation_record {
    LPVOID fiber{nullptr};
    stack_context sctx{};
    bool main_ctx{true};
    activation_record *from{nullptr};
    std::function<activation_record*(activation_record *&)> ontop{};
    bool terminated{false};
    bool force_unwind{false};

    static activation_record *&current() noexcept;

    // used for toplevel-context
    // (e.g. main context, thread-entry context)
    activation_record() noexcept {
#if ( _WIN32_WINNT > 0x0600)
      if (::IsThreadAFiber()) {
        fiber = ::GetCurrentFiber();
      } else {
        fiber = ::ConvertThreadToFiber(nullptr);
      }
#else
      fiber = ::ConvertThreadToFiber(nullptr);
      if (nullptr == fiber) {
        DWORD err = ::GetLastError();
        ENGINE_STATIC_ASSERT(ERROR_ALREADY_FIBER == err);
        fiber = ::GetCurrentFiber();
        ENGINE_STATIC_ASSERT(nullptr != fiber);
        ENGINE_STATIC_ASSERT(reinterpret_cast<LPVOID>(0x1E00) != fiber);
      }
#endif
    }

    activation_record(stack_context sctx_) noexcept: sctx{sctx_},
                                                     main_ctx{false} {
    }

    virtual ~activation_record() {
      if (main_ctx) {
        ::ConvertFiberToThread();
      } else {
        ::DeleteFiber(fiber);
      }
    }

    activation_record(activation_record const &) = delete;

    activation_record &operator=(activation_record const &) = delete;

    bool is_main_context() const noexcept {
      return main_ctx;
    }

    activation_record *resume() {
      from = current();
      // store `this` in static, thread local pointer
      // `this` will become the active (running) context
      current() = this;
      // context switch from parent context to `this`-context
      // context switch
      ::SwitchToFiber(fiber);
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
      return Core::Context::detail::exchange(current()->from, nullptr);
#else
      return std::exchange(current()->from, nullptr);
#endif
    }

    template<typename Ctx, typename Fn>
    activation_record *resume_with(Fn &&fn) {
      from = current();
      // store `this` in static, thread local pointer
      // `this` will become the active (running) context
      // returned by continuation::current()
      current() = this;
#if defined(BOOST_NO_CXX14_GENERIC_LAMBDAS)
      current()->ontop = std::bind(
        [](typename std::decay<Fn>::type &fn, activation_record *&ptr) {
          Ctx c{ptr};
          c = fn(std::move(c));
          if (!c) {
            ptr = nullptr;
          }
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
      return exchange(c.ptr_, nullptr);
#else
      return std::exchange(c.ptr_, nullptr);
#endif
                },
      std::forward<Fn>(fn),
          std::placeholders::_1);
#else
      current()->ontop = [fn=std::forward<Fn>(fn)](activation_record *&ptr) {
        Ctx c{ptr};
        c = fn(std::move(c));
        if (!c) {
          ptr = nullptr;
        }
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
        return exchange(c.ptr_, nullptr);
#else
        return std::exchange(c.ptr_, nullptr);
#endif
      };
#endif
      // context switch
      ::SwitchToFiber(fiber);
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
      return Core::Context::detail::exchange(current()->from, nullptr);
#else
      return std::exchange(current()->from, nullptr);
#endif
    }

    virtual void deallocate() noexcept {
    }
  };

  struct activation_record_initializer {
    activation_record_initializer() noexcept;

    ~activation_record_initializer();
  };

  struct forced_unwind {
    activation_record *from{nullptr};

    explicit forced_unwind(activation_record *from_): from{from_} {
    }
  };

  template<typename Ctx, typename StackAlloc, typename Fn>
  class capture_record: public activation_record {
    private:
      typename std::decay<StackAlloc>::type salloc_;
      typename std::decay<Fn>::type fn_;

      static void destroy(capture_record *p) noexcept {
        typename std::decay<StackAlloc>::type salloc = std::move(p->salloc_);
        stack_context sctx = p->sctx;
        // deallocate activation record
        p->~capture_record();
        // destroy stack with stack allocator
        salloc.deallocate(sctx);
      }

    public:
      capture_record(stack_context sctx, StackAlloc &&salloc, Fn &&fn) noexcept: activation_record(sctx),
        salloc_(std::forward<StackAlloc>(salloc)),
        fn_(std::forward<Fn>(fn)) {
      }

      void deallocate() noexcept override final {
        ENGINE_STATIC_ASSERT(main_ctx || (!main_ctx && terminated));
        destroy(this);
      }

      void run() {
        Ctx c{from};
        try {
          // invoke context-function
#if defined(BOOST_NO_CXX17_STD_INVOKE)
          c = Core::Context::Core::Context::detail::invoke(fn_, std::move(c));
#else
          c = std::invoke(fn_, std::move(c));
#endif
        } catch (forced_unwind const &ex) {
          c = Ctx{ex.from};
        }
        // this context has finished its task
        from = nullptr;
        ontop = nullptr;
        terminated = true;
        force_unwind = false;
        c.resume();
        ENGINE_ASSERT(false, "continuation already terminated");
      }
  };

  template<typename Ctx, typename StackAlloc, typename Fn>
  static activation_record *create_context1(StackAlloc &&salloc, Fn &&fn) {
    typedef capture_record<Ctx, StackAlloc, Fn> capture_t;

    auto sctx = salloc.allocate();
    BOOST_ASSERT((sizeof(capture_t)) < sctx.size);
    // reserve space for control structure
    void *storage = reinterpret_cast<void *>(
      (reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sizeof(capture_t)))
      & ~static_cast<uintptr_t>(0xff));
    // placement new for control structure on context stack
    capture_t *record = new(storage) capture_t{
      sctx, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn)
    };
    // create user-context
    record->fiber = ::CreateFiber(sctx.size, &Core::Context::detail::entry_func<capture_t>, record);
    return record;
  }

  template<typename Ctx, typename StackAlloc, typename Fn>
  static activation_record *create_context2(preallocated palloc, StackAlloc &&salloc, Fn &&fn) {
    typedef capture_record<Ctx, StackAlloc, Fn> capture_t;

    ENGINE_STATIC_ASSERT((sizeof(capture_t)) < palloc.size);
    // reserve space for control structure
    void *storage = reinterpret_cast<void *>(
      (reinterpret_cast<uintptr_t>(palloc.sp) - static_cast<uintptr_t>(sizeof(capture_t)))
      & ~static_cast<uintptr_t>(0xff));
    // placement new for control structure on context stack
    capture_t *record = new(storage) capture_t{
      palloc.sctx, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn)
    };
    // create user-context
    record->fiber = ::CreateFiber(palloc.sctx.size, &Core::Context::detail::entry_func<capture_t>, record);
    return record;
  }
}

namespace detail {
  class continuation {
    private:
      friend struct Core::Context::detail::activation_record;

      template<typename Ctx, typename StackAlloc, typename Fn>
      friend class Core::Context::detail::capture_record;

      template<typename Ctx, typename StackAlloc, typename Fn>
      friend Core::Context::detail::activation_record *Core::Context::detail::create_context1(StackAlloc &&, Fn &&);

      template<typename Ctx, typename StackAlloc, typename Fn>
      friend Core::Context::detail::activation_record *Core::Context::detail::create_context2(
        Core::Context::preallocated, StackAlloc &&, Fn &&);

      template<typename StackAlloc, typename Fn>
      friend continuation callcc(std::allocator_arg_t, StackAlloc &&, Fn &&);

      template<typename StackAlloc, typename Fn>
      friend continuation callcc(std::allocator_arg_t, Core::Context::preallocated, StackAlloc &&, Fn &&);

      Core::Context::detail::activation_record *ptr_{nullptr};

      continuation(Core::Context::detail::activation_record *ptr) noexcept: ptr_{ptr} {
      }

    public:
      continuation() = default;

      ~continuation() {
        if (nullptr != ptr_ && !ptr_->main_ctx) {
          if (!ptr_->terminated) {
            ptr_->force_unwind = true;
            ptr_->resume();
            ENGINE_ASSERT(ptr_->terminated, "");
          }
          ptr_->deallocate();
        }
      }

      continuation(continuation const &) = delete;

      continuation &operator=(continuation const &) = delete;

      continuation(continuation &&other) noexcept {
        swap(other);
      }

      continuation &operator=(continuation &&other) noexcept {
        if (this != &other) {
          continuation tmp = std::move(other);
          swap(tmp);
        }
        return *this;
      }

      continuation resume() & {
        return std::move(*this).resume();
      }

      continuation resume() && {
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
        Core::Context::detail::activation_record *ptr = Core::Context::detail::exchange(ptr_, nullptr)->resume();
#else
        Core::Context::detail::activation_record *ptr = std::exchange(ptr_, nullptr)->resume();
#endif
        if (Core::Context::detail::activation_record::current()->force_unwind) {
          throw Core::Context::detail::forced_unwind{ptr};
        } else if (nullptr != Core::Context::detail::activation_record::current()->ontop) {
          ptr = Core::Context::detail::activation_record::current()->ontop(ptr);
          Core::Context::detail::activation_record::current()->ontop = nullptr;
        }
        return {ptr};
      }

      template<typename Fn>
      continuation resume_with(Fn &&fn) & {
        return std::move(*this).resume_with(std::forward<Fn>(fn));
      }

      template<typename Fn>
      continuation resume_with(Fn &&fn) && {
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
        Core::Context::detail::activation_record *ptr =
            Core::Context::detail::exchange(ptr_, nullptr)->resume_with<continuation>(std::forward<Fn>(fn));
#else
        Core::Context::Core::Context::detail::activation_record *ptr =
            std::exchange(ptr_, nullptr)->resume_with<continuation>(std::forward<Fn>(fn));
#endif
        if (Core::Context::detail::activation_record::current()->force_unwind) {
          throw Core::Context::detail::forced_unwind{ptr};
        } else if (nullptr != Core::Context::detail::activation_record::current()->ontop) {
          ptr = Core::Context::detail::activation_record::current()->ontop(ptr);
          Core::Context::detail::activation_record::current()->ontop = nullptr;
        }
        return {ptr};
      }

      explicit operator bool() const noexcept {
        return nullptr != ptr_ && !ptr_->terminated;
      }

      bool operator!() const noexcept {
        return nullptr == ptr_ || ptr_->terminated;
      }

      bool operator<(continuation const &other) const noexcept {
        return ptr_ < other.ptr_;
      }

#if !defined(BOOST_EMBTC)

      template<typename charT, class traitsT>
      friend std::basic_ostream<charT, traitsT> &
      operator<<(std::basic_ostream<charT, traitsT> &os, continuation const &other) {
        if (nullptr != other.ptr_) {
          return os << other.ptr_;
        } else {
          return os << "{not-a-context}";
        }
      }

#else

      template<typename charT, class traitsT>
      friend std::basic_ostream<charT, traitsT> &operator<<(std::basic_ostream<charT, traitsT> &os,
                                                            continuation const &other);

#endif

      void swap(continuation &other) noexcept {
        std::swap(ptr_, other.ptr_);
      }


#if defined(BOOST_EMBTC)

      template<typename charT, class traitsT>
      inline std::basic_ostream<charT, traitsT> &operator
      <<(std::basic_ostream<charT, traitsT> &os, continuation const &other) {
        if (nullptr != other.ptr_) {
          return os << other.ptr_;
        } else {
          return os << "{not-a-context}";
        }
      }

#endif

      template<
        typename Fn,
        typename = Core::Context::detail::disable_overload<continuation, Fn> >
      continuation callcc(Fn &&fn) {
        return callcc(
          std::allocator_arg,
          Core::Fibers::fixedsize_stack(),
          std::forward<Fn>(fn));
      }

      template<typename StackAlloc, typename Fn>
      continuation callcc(std::allocator_arg_t, StackAlloc &&salloc, Fn &&fn) {
        return continuation{
          Core::Context::detail::create_context1<continuation>(
            std::forward<StackAlloc>(salloc), std::forward<Fn>(fn))
        }.resume();
      }

      template<typename StackAlloc, typename Fn>
      continuation callcc(std::allocator_arg_t, Core::Context::preallocated palloc, StackAlloc &&salloc, Fn &&fn) {
        return continuation{
          Core::Context::detail::create_context2<continuation>(
            palloc, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn))
        }.resume();
      }

      inline void swap(continuation &l, continuation &r) noexcept {
        l.swap(r);
      }
  };
};

#if defined(ENGINE_COMPILER_MSVC)
#pragma warning(pop)
#endif
