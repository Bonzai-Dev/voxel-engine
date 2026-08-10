#pragma once

#include <cstddef>
#include "detail/config.hpp"

namespace Core::Context {
  struct preallocated {
    void *sp;
    std::size_t size;
    stack_context sctx;

    preallocated(void *sp_, std::size_t size_, stack_context sctx_) noexcept: sp(sp_), size(size_), sctx(sctx_) {
    }
  };
}
