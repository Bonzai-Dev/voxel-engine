#include <core/threading/fiber/context.hpp>
#include "algorithm.hpp"

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core::Fibers::algo {
  //static
  fiber_properties *algorithm_with_properties_base::get_properties(context *ctx) noexcept {
    return ctx->get_properties();
  }

  //static
  void algorithm_with_properties_base::set_properties(context *ctx, fiber_properties *props) noexcept {
    ctx->set_properties(props);
  }
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
