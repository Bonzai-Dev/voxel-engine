#pragma once
// #include <boost/config.hpp>

#if defined(BOOST_USE_SEGMENTED_STACKS)
# if !defined(ENGINE_PLATFORM_WINDOWS)
#include <boost/context/posix/segmented_stack.hpp>
# endif
#endif
