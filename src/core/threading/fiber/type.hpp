#pragma once
#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>

#include <core/assert.hpp>
// #include <boost/config.hpp>
#include <core/threading/context/detail/apply.hpp>
#include <core/threading/context/stack_context.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/parent_from_member.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/intrusive/set.hpp>

#include <core/threading/fiber/detail/config.hpp>
#include <core/threading/fiber/detail/data.hpp>
#include <core/threading/fiber/detail/decay_copy.hpp>
#include <core/threading/fiber/detail/fss.hpp>
#include <core/threading/fiber/detail/spinlock.hpp>
#include <core/threading/fiber/exceptions.hpp>
#include <core/threading/fiber/fixedsize_stack.hpp>
#include <core/threading/fiber/properties.hpp>
#include <core/threading/fiber/segmented_stack.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core {
  namespace Fibers {
    enum class type {
      none = 0,
      main_context = 1 << 1,
      dispatcher_context = 1 << 2,
      worker_context = 1 << 3,
      pinned_context = main_context | dispatcher_context
    };

    inline constexpr type operator&(type l, type r) {
      return static_cast<type>(
        static_cast<unsigned int>(l) & static_cast<unsigned int>(r));
    }

    inline constexpr type operator|(type l, type r) {
      return static_cast<type>(
        static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
    }

    inline  constexpr type operator^(type l, type r) {
      return static_cast<type>(
        static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r));
    }

    inline constexpr type operator~(type l) {
      return static_cast<type>(~static_cast<unsigned int>(l));
    }

    inline type &operator&=(type &l, type r) {
      l = l & r;
      return l;
    }

    inline type &operator|=(type &l, type r) {
      l = l | r;
      return l;
    }

    inline type &operator^=(type &l, type r) {
      l = l ^ r;
      return l;
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
