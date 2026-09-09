//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
// #include <boost/config.hpp>

#if defined(BOOST_USE_SEGMENTED_STACKS)
# if !defined(ENGINE_PLATFORM_WINDOWS)
#  include <core/threading/context/posix/segmented_stack.hpp>
# endif
#endif
