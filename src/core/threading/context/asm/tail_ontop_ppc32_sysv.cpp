#include "../detail/fcontext.hpp"

using Core::Context::detail::fcontext_t;
using Core::Context::detail::transfer_t;

// This C++ tail of ontop_fcontext() allocates transfer_t{ from, vp }
// on the stack.  If fn() throws a C++ exception, then the C++ runtime
// must remove this tail's stack frame.
extern "C" transfer_t ontop_fcontext_tail(int ignore, void *vp, transfer_t (*fn)(transfer_t), fcontext_t const from) {
  return fn(transfer_t{from, vp});
}
