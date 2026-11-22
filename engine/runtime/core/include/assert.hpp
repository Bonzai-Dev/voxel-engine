#pragma once

#if defined(ENGINE_PLATFORM_WINDOWS)
  #define ENGINE_DEBUG_BREAK() __debugbreak()
#elif defined(ENGINE_COMPILER_CLANG)
  #define ENGINE_DEBUG_BREAK() __builtin_debugtrap()
#elif defined(ENGINE_COMPILER_GCC)
  #include <csignal>
  #define ENGINE_DEBUG_BREAK() raise(SIGTRAP)
#else
  #define ENGINE_DEBUG_BREAK()
#endif

#if defined(ENGINE_DEBUG)
  #define ENGINE_ENABLE_ASSERTS
#endif

#if defined(ENGINE_ENABLE_ASSERTS)
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
