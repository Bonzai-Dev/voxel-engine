#pragma once

#define ENGINE_BIT(x) (1u << x)

#define ENGINE_ENUM_CLASS_FLAGS(name, type, ...) \
enum class name: type; \
constexpr name operator ~ (name val) { return (name)(~(type)val); } \
constexpr type operator & (name val0, name val1) { return (type)val0 & (type)val1; } \
constexpr name operator | (name val0, name val1) { return (name)((type)val0 | (type)val1); } \
constexpr name& operator &= (name& val0, name val1) { val0 = (name)(val0 & val1); return val0; } \
constexpr name& operator |= (name& val0, name val1) { val0 = (name)(val0 | val1); return val0; } \
enum class name : type { __VA_ARGS__ }
