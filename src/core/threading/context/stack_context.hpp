#pragma once

#include <cstddef>
#include <boost/config.hpp>
// #include <core/threading/context/detail/config.hpp>

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_PREFIX
// #endif

namespace Core::Context {
#if ! defined(BOOST_CONTEXT_NO_CXX11)
  struct stack_context {
# if defined(BOOST_USE_SEGMENTED_STACKS)
    typedef void *segments_context[BOOST_CONTEXT_SEGMENTS];
# endif

    std::size_t size{0};
    void *sp{nullptr};
# if defined(BOOST_USE_SEGMENTED_STACKS)
    segments_context segments_ctx{};
# endif
# if defined(BOOST_USE_VALGRIND)
    unsigned valgrind_stack_id{0};
# endif
  };
#else
  struct BOOST_CONTEXT_DECL stack_context{
# if defined(BOOST_USE_SEGMENTED_STACKS)
  typedef void *segments_context[BOOST_CONTEXT_SEGMENTS];
# endif

  std::size_t size;
  void *sp;
# if defined(BOOST_USE_SEGMENTED_STACKS)
  segments_context segments_ctx;
# endif
# if defined(BOOST_USE_VALGRIND)
  unsigned valgrind_stack_id;
# endif

  stack_context():
    size(0),
    sp(0)
# if defined(BOOST_USE_SEGMENTED_STACKS)
  , segments_ctx()
# endif
# if defined(BOOST_USE_VALGRIND)
  , valgrind_stack_id (0)
# endif
  {}
};
#endif
}

// #ifdef BOOST_HAS_ABI_HEADERS
// #  include BOOST_ABI_SUFFIX
// #endif
