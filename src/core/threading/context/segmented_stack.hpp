#pragma once

#if defined(BOOST_USE_SEGMENTED_STACKS)
# if !defined(ENGINE_PLATFORM_WINDOWS)
#include "posix/segmented_stack.hpp"
# endif
#endif
