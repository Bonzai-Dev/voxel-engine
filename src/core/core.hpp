#pragma once

/* clang-format off */
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

#define ENGINE_BIT(x) (1u << x)

#define ENGINE_ENUM_BITS(name, type, ...) \
  enum class name : type; \
  constexpr name operator ~ (name val) { return (name)(~(type)val); } \
  constexpr type operator & (name val0, name val1) { return (type)val0 & (type)val1; } \
  constexpr name operator | (name val0, name val1) { return (name)((type)val0 | (type)val1); } \
  constexpr name& operator &= (name& val0, name val1) { val0 = (name)(val0 & val1); return val0; } \
  constexpr name& operator |= (name& val0, name val1) { val0 = (name)(val0 | val1); return val0; } \
  enum class name : type { __VA_ARGS__ }
/* clang-format on */
