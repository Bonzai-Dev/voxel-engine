//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
// #include <boost/config.hpp>
#include <core/threading/context/detail/config.hpp>

namespace boost::context {
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
}
