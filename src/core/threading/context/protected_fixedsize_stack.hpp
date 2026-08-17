#pragma once
// #include <boost/config.hpp>

#if defined(ENGINE_PLATFORM_WINDOWS)
# include <core/threading/context/windows/protected_fixedsize_stack.hpp>
#else
# include <core/threading/context/posix/protected_fixedsize_stack.hpp>
#endif
