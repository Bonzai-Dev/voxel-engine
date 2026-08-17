#pragma once
#include <memory>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/context/stack_context.hpp>
#include <core/threading/fiber/fixedsize_stack.hpp>
#include <core/threading/fiber/segmented_stack.hpp>

namespace Core {
  namespace Fibers {
    namespace detail {
      class polymorphic_stack_allocator_base {
        public:
          polymorphic_stack_allocator_base() = default;

          virtual ~polymorphic_stack_allocator_base() = default;

          polymorphic_stack_allocator_base(const polymorphic_stack_allocator_base &) = delete;
          polymorphic_stack_allocator_base &operator=(const polymorphic_stack_allocator_base &) = delete;

          polymorphic_stack_allocator_base(polymorphic_stack_allocator_base &&) = delete;
          polymorphic_stack_allocator_base &operator=(polymorphic_stack_allocator_base &&) = delete;

          virtual Core::Context::stack_context allocate() = 0;

          virtual void deallocate(Core::Context::stack_context &sctx) = 0;
      };

      template <typename StackAllocator>
      class polymorphic_stack_allocator_impl final: public polymorphic_stack_allocator_base {
        public:
          template <typename... Args>
          polymorphic_stack_allocator_impl(Args &&... args)
            : _allocator(std::forward<Args>(args)...) {
          }

          ~polymorphic_stack_allocator_impl() = default;

          Core::Context::stack_context allocate() override {
            return _allocator.allocate();
          }

          void deallocate(Core::Context::stack_context &sctx) override {
            _allocator.deallocate(sctx);
          }

        private:
          StackAllocator _allocator;
      };
    }

    class BOOST_FIBERS_DECL stack_allocator_wrapper final {
      public:
        stack_allocator_wrapper(std::unique_ptr<detail::polymorphic_stack_allocator_base> allocator)
          : _allocator(std::move(allocator)) {
        }

        ~stack_allocator_wrapper() = default;

        stack_allocator_wrapper(const stack_allocator_wrapper &) = delete;
        stack_allocator_wrapper &operator=(const stack_allocator_wrapper &) = delete;

        stack_allocator_wrapper(stack_allocator_wrapper &&) = default;
        stack_allocator_wrapper &operator=(stack_allocator_wrapper &&) = default;

        Core::Context::stack_context allocate() {
          return _allocator->allocate();
        }

        void deallocate(Core::Context::stack_context &sctx) {
          _allocator->deallocate(sctx);
        }

      private:
        std::unique_ptr<detail::polymorphic_stack_allocator_base> _allocator;
    };

    template <typename StackAllocator, typename... Args>
    stack_allocator_wrapper make_stack_allocator_wrapper(Args &&... args) {
      return stack_allocator_wrapper(
        std::unique_ptr<detail::polymorphic_stack_allocator_base>(
          new detail::polymorphic_stack_allocator_impl<StackAllocator>(std::forward<Args>(args)...)));
    }
  }
}
