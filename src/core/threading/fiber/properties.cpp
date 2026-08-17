#include <core/assert.hpp>
#include "properties.hpp"
#include <core/threading/fiber/algo/algorithm.hpp>
#include <core/threading/fiber/scheduler.hpp>
#include <core/threading/fiber/context.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace Core {
  namespace Fibers {
    void fiber_properties::notify() noexcept {
      ENGINE_ASSERT(nullptr != algo_, "");
      // Application code might change an important property for any fiber at
      // any time. The fiber in question might be ready, running or waiting.
      // Significantly, only a fiber which is ready but not actually running is
      // in the sched_algorithm's ready queue. Don't bother the sched_algorithm
      // with a change to a fiber it's not currently tracking: it will do the
      // right thing next time the fiber is passed to its awakened() method.
      if (ctx_->ready_is_linked()) {
        dynamic_cast<algo::algorithm_with_properties_base*>(algo_) -> property_change_(ctx_, this);
      }
    }
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
