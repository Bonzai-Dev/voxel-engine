#pragma once

#include <memory>
// #include <boost/config.hpp>
#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/future/detail/shared_state.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    namespace detail {
      template <typename R, typename Allocator>
      class shared_state_object: public shared_state<R> {
        public:
          typedef typename std::allocator_traits<Allocator>::template rebind_alloc<
            shared_state_object
          > allocator_type;

          shared_state_object(allocator_type const &alloc):
            shared_state<R>{},
            alloc_{alloc} {
          }

        protected:
          void deallocate_future() noexcept override final {
            destroy_(alloc_, this);
          }

        private:
          allocator_type alloc_;

          static void destroy_(allocator_type const &alloc, shared_state_object *p) noexcept {
            allocator_type a{alloc};
            typedef std::allocator_traits<allocator_type> traity_type;
            traity_type::destroy(a, p);
            traity_type::deallocate(a, p, 1);
          }
      };
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
