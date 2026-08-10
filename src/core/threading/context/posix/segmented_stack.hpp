#pragma once
#include <cstddef>
#include <new>
#include "../detail/config.hpp"
#include "../stack_context.hpp"
#include "../stack_traits.hpp"

// forward declaration for splitstack-functions defined in libgcc
extern "C" {
void *__splitstack_makecontext(std::size_t,
                               void *[BOOST_CONTEXT_SEGMENTS],
                               std::size_t *);

void __splitstack_releasecontext(void *[BOOST_CONTEXT_SEGMENTS]);

void __splitstack_resetcontext(void *[BOOST_CONTEXT_SEGMENTS]);

void __splitstack_block_signals_context(void *[BOOST_CONTEXT_SEGMENTS],
                                        int *new_value, int *old_value);
}

namespace Core::Context {
  template<typename traitsT>
  class basic_segmented_stack {
    public:
      typedef traitsT traits_type;

      basic_segmented_stack(std::size_t size = traits_type::default_size()) noexcept: size_(size) {
      }

      stack_context allocate() {
        stack_context sctx;
        void *vp = __splitstack_makecontext(size_, sctx.segments_ctx, &sctx.size);
        if (!vp) throw std::bad_alloc();

        // sctx.size is already filled by __splitstack_makecontext
        sctx.sp = static_cast<char *>(vp) + sctx.size;

        int off = 0;
        __splitstack_block_signals_context(sctx.segments_ctx, &off, 0);

        return sctx;
      }

      void deallocate(stack_context &sctx) noexcept {
        __splitstack_releasecontext(sctx.segments_ctx);
      }

    private:
      std::size_t size_;
  };

  typedef basic_segmented_stack<stack_traits> segmented_stack;
# if defined(BOOST_USE_SEGMENTED_STACKS)
  typedef segmented_stack default_stack;
# endif
}
