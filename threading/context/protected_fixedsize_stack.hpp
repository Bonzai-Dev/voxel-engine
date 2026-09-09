//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
// #include <boost/config.hpp>

#if defined(ENGINE_PLATFORM_WINDOWS)
# include <core/threading/context/windows/protected_fixedsize_stack.hpp>
#else
# include <core/threading/context/posix/protected_fixedsize_stack.hpp>
#endif
