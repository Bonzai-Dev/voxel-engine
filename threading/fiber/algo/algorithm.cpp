//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "algorithm.hpp"
#include "../context.hpp"
#include <core/threading/fiber/properties.hpp>

namespace boost::fibers::algo {
  //static
  fiber_properties *algorithm_with_properties_base::get_properties(context *ctx) noexcept {
    return ctx->get_properties();
  }

  //static
  void algorithm_with_properties_base::set_properties(context *ctx, fiber_properties *props) noexcept {
    ctx->set_properties(props);
  }
}
