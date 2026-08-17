#include <core/threading/context/detail/fcontext.hpp>

extern "C"
Core::Context::detail::transfer_t BOOST_CONTEXT_CALLDECL jump_fcontext(
  Core::Context::detail::fcontext_t const to, void *vp
);

extern "C"
Core::Context::detail::fcontext_t BOOST_CONTEXT_CALLDECL make_fcontext(
  void *sp, std::size_t size, void (*fn)(Core::Context::detail::transfer_t)
);

// based on an idea of Giovanni Derreta
extern "C"
Core::Context::detail::transfer_t BOOST_CONTEXT_CALLDECL ontop_fcontext(
  Core::Context::detail::fcontext_t const to, void *vp,
  Core::Context::detail::transfer_t (*fn)(Core::Context::detail::transfer_t)
);

namespace Core::Context::detail {
  transfer_t jump_fcontext(fcontext_t const to, void *vp) {
    return ::jump_fcontext(to, vp);
  }

  fcontext_t make_fcontext(void *sp, std::size_t size, void (*fn)(transfer_t)) {
    return ::make_fcontext(sp, size, fn);
  }

  transfer_t ontop_fcontext(fcontext_t const to, void *vp, transfer_t (*fn)(transfer_t)) {
    return ::ontop_fcontext(to, vp, fn);
  }
}
