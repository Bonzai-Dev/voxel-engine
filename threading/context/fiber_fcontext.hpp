//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/threading/context/detail/config.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <memory>
#include <ostream>
#include <tuple>
#include <utility>

#include <core/assert.hpp>
// #include <boost/config.hpp>
#include <core/memory/intrusive_ptr.hpp>

#include <core/threading/context/detail/disable_overload.hpp>
#include <core/threading/context/detail/exception.hpp>
#include <core/threading/context/detail/fcontext.hpp>
#include <core/threading/context/detail/tuple.hpp>
#include <core/threading/context/fixedsize_stack.hpp>
#include <core/threading/context/flags.hpp>
#include <core/threading/context/preallocated.hpp>
#include <core/threading/context/segmented_stack.hpp>
#include <core/threading/context/stack_context.hpp>

#include "core/application.hpp"

#if defined(__CET__) && defined(__unix__)
#  include <cet.h>
#  include <sys/mman.h>
#  include <unistd.h>
#  define SHSTK_ENABLED (__CET__ & 0x2)
#  define BOOST_CONTEXT_SHADOW_STACK (SHSTK_ENABLED && SHADOW_STACK_SYSCALL)
#  if !defined(__NR_map_shadow_stack)
#    define __NR_map_shadow_stack 453
#  endif
#ifndef SHADOW_STACK_SET_TOKEN
#  define SHADOW_STACK_SET_TOKEN 0x1
#endif
#endif

#if defined(ENGINE_COMPILER_MSVC)
# pragma warning(push)
# pragma warning(disable: 4702)
#endif

#if ! (defined(__GLIBCPP__) || defined(__GLIBCXX__))
namespace boost {
  namespace context {
    namespace detail {
      // manage_exception_state is a dummy struct unless we have specific support
      struct manage_exception_state {
        void from_forced_unwind() noexcept {
        }
      };
    } // namespace detail
  } // namespace context
} // namespace boost

#else // libstdc++
#include <cxxabi.h>

namespace __cxxabiv1 {
  struct __cxa_eh_globals {
    void *caughtExceptions;
    unsigned int uncaughtExceptions;
  };

  class manage_exception_state {
    public:
      ENGINE_NO_INLINE manage_exception_state() {
        exception_state_ = *__cxa_get_globals();
      }

      // Hack to account for the forced_unwind exception thrown in fiber_unwind
      // that's run ontop before the destructor.
      void from_forced_unwind() noexcept {
        exception_state_.uncaughtExceptions += 1;
      }

      ENGINE_NO_INLINE ~manage_exception_state() {
        *__cxa_get_globals() = exception_state_;
      }

    private:
      __cxa_eh_globals exception_state_;
  };
} // namespace __cxxabiv1


namespace boost::context::detail {
  using __cxxabiv1::manage_exception_state;
} // namespace boost::context::detail


#endif // __GLIBCPP__ or __GLIBCXX__

namespace boost::context {
  namespace detail {
    inline
    transfer_t fiber_unwind(transfer_t t) {
      throw forced_unwind(t.fctx);
      return {nullptr, nullptr};
    }

    template <typename Rec>
    transfer_t fiber_exit(transfer_t t) noexcept {
      Rec *rec = static_cast<Rec*>(t.data);
#if BOOST_CONTEXT_SHADOW_STACK
      // destroy shadow stack
      std::size_t ss_size = *((unsigned long*)(reinterpret_cast<uintptr_t>(rec) - 16));
      long unsigned int ss_base = *((unsigned long*)(reinterpret_cast<uintptr_t>(rec) - 8));
      munmap((void*)ss_base, ss_size);
#endif
      // destroy context stack
      rec->deallocate();
      return {nullptr, nullptr};
    }

    template <typename Rec>
    void fiber_entry(transfer_t t) noexcept {
      // transfer control structure to the context-stack
      Rec *rec = static_cast<Rec*>(t.data);
      ENGINE_ASSERT(nullptr != t.fctx, "");
      ENGINE_ASSERT(nullptr != rec, "");
      try {
        // jump back to `create_context()`
        t = jump_fcontext(t.fctx, nullptr);
        // start executing
        t.fctx = rec->run(t.fctx);
      }
      catch (forced_unwind const &ex) {
        t = {ex.fctx, nullptr};
      }
      ENGINE_ASSERT(nullptr != t.fctx, "");
      // destroy context-stack of `this`context on next context
      ontop_fcontext(t.fctx, rec, fiber_exit<Rec>);
      ENGINE_ASSERT(false, "context already terminated");
    }

    template <typename Ctx, typename Fn>
    transfer_t fiber_ontop(transfer_t t) {
      ENGINE_ASSERT(nullptr != t.data, "");
      auto p = *static_cast<Fn*>(t.data);
      t.data = nullptr;
      // execute function, pass fiber via reference
      Ctx c = p(Ctx{t.fctx});
      return {std::exchange(c.fctx_, nullptr), nullptr};
    }

    template <typename Ctx, typename StackAlloc, typename Fn>
    class fiber_record {
      private:
        stack_context sctx_;
        typename std::decay<StackAlloc>::type salloc_;
        typename std::decay<Fn>::type fn_;

        static void destroy(fiber_record *p) noexcept {
          typename std::decay<StackAlloc>::type salloc = std::move(p->salloc_);
          stack_context sctx = p->sctx_;
          // deallocate fiber_record
          p->~fiber_record();
          // destroy stack with stack allocator
          salloc.deallocate(sctx);
        }

      public:
        fiber_record(stack_context sctx, StackAlloc &&salloc,
                     Fn &&fn) noexcept :
          sctx_(sctx),
          salloc_(std::forward<StackAlloc>(salloc)),
          fn_(std::forward<Fn>(fn)) {
        }

        fiber_record(fiber_record const &) = delete;
        fiber_record &operator=(fiber_record const &) = delete;

        void deallocate() noexcept {
          destroy(this);
        }

        fcontext_t run(fcontext_t fctx) {
          // invoke context-function
          Ctx c = std::invoke(fn_, Ctx{fctx});
          return std::exchange(c.fctx_, nullptr);
        }
    };

    template <typename Record, typename StackAlloc, typename Fn>
    fcontext_t create_fiber1(StackAlloc &&salloc, Fn &&fn) {
      auto sctx = salloc.allocate();
      // reserve space for control structure
      void *storage = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sizeof(Record)))
        & ~static_cast<uintptr_t>(0xff));
      // placement new for control structure on context stack
      Record *record = new(storage) Record{
        sctx, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn)
      };
      // 64byte gab between control structure and stack top
      // should be 16byte aligned
      void *stack_top = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
      void *stack_bottom = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(sctx.sp) - static_cast<uintptr_t>(sctx.size));
      // create fast-context
      const std::size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);

#if BOOST_CONTEXT_SHADOW_STACK
      std::size_t ss_size = size >> 5;
      // align shadow stack to 8 bytes.
      ss_size = (ss_size + 7) & ~7;
      // Todo: shadow stack occupies at least 4KB
      ss_size = (ss_size > 4096) ? size : 4096;
      // create shadow stack
      void *ss_base = (void*)syscall(__NR_map_shadow_stack, 0, ss_size, SHADOW_STACK_SET_TOKEN);
      ENGINE_ASSERT(ss_base != -1, "");
      unsigned long ss_sp = (unsigned long)ss_base + ss_size;
      /* pass the shadow stack pointer to make_fcontext
         i.e., link the new shadow stack with the new fcontext
         TODO should be a better way? */
      *((unsigned long*)(reinterpret_cast<uintptr_t>(stack_top) - 8)) = ss_sp;
      /* Todo: place shadow stack info in 64byte gap */
      *((unsigned long*)(reinterpret_cast<uintptr_t>(storage) - 8)) = (unsigned long)ss_base;
      *((unsigned long*)(reinterpret_cast<uintptr_t>(storage) - 16)) = ss_size;
#endif
      const fcontext_t fctx = make_fcontext(stack_top, size, &fiber_entry<Record>);
      ENGINE_ASSERT(nullptr != fctx, "");
      // transfer control structure to context-stack
      return jump_fcontext(fctx, record).fctx;
    }

    template <typename Record, typename StackAlloc, typename Fn>
    fcontext_t create_fiber2(preallocated palloc, StackAlloc &&salloc, Fn &&fn) {
      // reserve space for control structure
      void *storage = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(palloc.sp) - static_cast<uintptr_t>(sizeof(Record)))
        & ~static_cast<uintptr_t>(0xff));
      // placwment new for control structure on context-stack
      Record *record = new(storage) Record{
        palloc.sctx, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn)
      };
      // 64byte gab between control structure and stack top
      void *stack_top = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
      void *stack_bottom = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(palloc.sctx.sp) - static_cast<uintptr_t>(palloc.sctx.size));
      // create fast-context
      const std::size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);

#if BOOST_CONTEXT_SHADOW_STACK
      std::size_t ss_size = size >> 5;
      // align shadow stack to 8 bytes.
      ss_size = (ss_size + 7) & ~7;
      // Todo: shadow stack occupies at least 4KB
      ss_size = (ss_size > 4096) ? size : 4096;
      // create shadow stack
      void *ss_base = (void*)syscall(__NR_map_shadow_stack, 0, ss_size, SHADOW_STACK_SET_TOKEN);
      ENGINE_ASSERT(ss_base != -1, "");
      unsigned long ss_sp = (unsigned long)ss_base + ss_size;
      /* pass the shadow stack pointer to make_fcontext
         i.e., link the new shadow stack with the new fcontext
         TODO should be a better way? */
      *((unsigned long*)(reinterpret_cast<uintptr_t>(stack_top) - 8)) = ss_sp;
      /* Todo: place shadow stack info in 64byte gap */
      *((unsigned long*)(reinterpret_cast<uintptr_t>(storage) - 8)) = (unsigned long)ss_base;
      *((unsigned long*)(reinterpret_cast<uintptr_t>(storage) - 16)) = ss_size;
#endif
      const fcontext_t fctx = make_fcontext(stack_top, size, &fiber_entry<Record>);
      ENGINE_ASSERT(nullptr != fctx, "");
      // transfer control structure to context-stack
      return jump_fcontext(fctx, record).fctx;
    }
  } // namespace detail

  class fiber {
    private:
      template <typename Ctx, typename StackAlloc, typename Fn>
      friend class detail::fiber_record;

      template <typename Ctx, typename Fn>
      friend detail::transfer_t
      detail::fiber_ontop(detail::transfer_t);

      detail::fcontext_t fctx_{nullptr};

      fiber(detail::fcontext_t fctx) noexcept :
        fctx_{fctx} {
      }

    public:
      fiber() noexcept = default;

      template <typename Fn, typename = detail::disable_overload<fiber, Fn>>
      fiber(Fn &&fn) :
        fiber{std::allocator_arg, fixedsize_stack(), std::forward<Fn>(fn)} {
      }

      template <typename StackAlloc, typename Fn>
      fiber(std::allocator_arg_t, StackAlloc &&salloc, Fn &&fn) :
        fctx_{
          detail::create_fiber1<detail::fiber_record<fiber, StackAlloc, Fn>>(
            std::forward<StackAlloc>(salloc), std::forward<Fn>(fn))
        } {
      }

      template <typename StackAlloc, typename Fn>
      fiber(std::allocator_arg_t, preallocated palloc, StackAlloc &&salloc, Fn &&fn) :
        fctx_{
          detail::create_fiber2<detail::fiber_record<fiber, StackAlloc, Fn>>(
            palloc, std::forward<StackAlloc>(salloc), std::forward<Fn>(fn))
        } {
      }

#if defined(BOOST_USE_SEGMENTED_STACKS)
      template <typename Fn>
      fiber(std::allocator_arg_t, segmented_stack, Fn &&);

      template <typename StackAlloc, typename Fn>
      fiber(std::allocator_arg_t, preallocated, segmented_stack, Fn &&);
#endif

      ~fiber() {
        if (nullptr != fctx_) [[unlikely]] {
          [[maybe_unused]] detail::manage_exception_state exstate;
          detail::ontop_fcontext(
            std::exchange(fctx_, nullptr),
            nullptr,
            detail::fiber_unwind);
        }
      }

      fiber(fiber &&other) noexcept {
        swap(other);
      }

      fiber &operator=(fiber &&other) noexcept {
        if (this != &other) [[likely]] {
          fiber tmp = std::move(other);
          swap(tmp);
        }
        return *this;
      }

      fiber(fiber const &other) noexcept = delete;
      fiber &operator=(fiber const &other) noexcept = delete;

      fiber resume() && {
        ENGINE_ASSERT(nullptr != fctx_, "");
        [[maybe_unused]] detail::manage_exception_state exstate;
        try {
          return {
            detail::jump_fcontext(
              std::exchange(fctx_, nullptr),
              nullptr).fctx
          };
        }
        catch (detail::forced_unwind const &) {
          exstate.from_forced_unwind();
          throw;
        }
      }

      template <typename Fn>
      fiber resume_with(Fn &&fn) && {
        ENGINE_ASSERT(nullptr != fctx_, "");
        [[maybe_unused]] detail::manage_exception_state exstate;
        auto p = std::forward<Fn>(fn);
        try {
          return {
            detail::ontop_fcontext(
              std::exchange(fctx_, nullptr),
              &p,
              detail::fiber_ontop<fiber, decltype(p)>).fctx
          };
        }
        catch (detail::forced_unwind const &) {
          exstate.from_forced_unwind();
          throw;
        }
      }

      explicit operator bool() const noexcept {
        return nullptr != fctx_;
      }

      bool operator!() const noexcept {
        return nullptr == fctx_;
      }

      bool operator<(fiber const &other) const noexcept {
        return fctx_ < other.fctx_;
      }

      template <typename charT, class traitsT>
      friend std::basic_ostream<charT, traitsT> &
      operator<<(std::basic_ostream<charT, traitsT> &os, fiber const &other) {
        if (nullptr != other.fctx_) {
          return os << other.fctx_;
        }
        else {
          return os << "{not-a-context}";
        }
      }

      void swap(fiber &other) noexcept {
        std::swap(fctx_, other.fctx_);
      }
  };

  inline void swap(fiber &l, fiber &r) noexcept {
    l.swap(r);
  }

  typedef fiber fiber_context;
}

#if defined(ENGINE_COMPILER_MSVC)
# pragma warning(pop)
#endif
