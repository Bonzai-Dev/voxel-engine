#pragma once

#include <core/assert.hpp>
#include "fcontext.hpp"

namespace Core::Context::detail {
  struct forced_unwind {
    fcontext_t fctx{nullptr};

    forced_unwind() = default;

    forced_unwind(fcontext_t fctx_): fctx(fctx_) {
    }
  };
}
