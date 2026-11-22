#pragma once

#if defined(ENGINE_COMPILER_MSVC)
  #define ENGINE_FORCE_INLINE __forceinline
  #define ENGINE_NO_INLINE __declspec(noinline)
#elif defined(ENGINE_COMPILER_GCC) && __GNUC__ > 3
  // Clang also defines __GNUC__ (as 4)
  #define ENGINE_FORCE_INLINE inline __attribute__ ((__always_inline__))
  #define ENGINE_NO_INLINE __attribute__ ((noinline))
#else
  #define ENGINE_FORCE_INLINE inline
  #define ENGINE_NO_INLINE
#endif
