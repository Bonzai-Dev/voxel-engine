#pragma once
#if defined(ENGINE_PLATFORM_WINDOWS)
# include "windows/protected_fixedsize_stack.hpp"
#else
# include "posix/protected_fixedsize_stack.hpp"
#endif
