#pragma once

#include <core/logger.hpp>

#ifdef ENGINE_PLATFORM_WINDOWS
  #define ENGINE_DEBUG_BREAK() __debugbreak()
#elif ENGINE_COMPILER_CLANG
  #define ENGINE_DEBUG_BREAK() __builtin_debugtrap()
# elif ENGINE_COMPILER_GCC // TODO: NEEDS TESTING
  #include <signal.h>
  #define ENGINE_DEBUG_BREAK() raise(SIGTRAP)
#else
  #define ENGINE_DEBUG_BREAK()
#endif

#ifdef ENGINE_DEBUG
  #define ENGINE_ENABLE_ASSERTS
#endif

#ifdef ENGINE_ENABLE_ASSERTS
  #define ENGINE_ASSERT(condition, ...) \
    if (condition) { } \
    else { \
      LOG_CORE_ERROR("Assertion failed: {0}\n\tExpression: {1}\n\tFile: {2}:{3}\n", __VA_ARGS__, #condition, __FILE__, __LINE__); \
      ENGINE_DEBUG_BREAK(); \
    }

  #define ENGINE_STATIC_ASSERT(condition, ...) static_assert(condition, __VA_ARGS__)
#else
  #define ENGINE_ASSERT(condition, ...)
  #define ENGINE_STATIC_ASSERT(condition, ...)
#endif
