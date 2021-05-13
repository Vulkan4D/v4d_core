#pragma once

#include <cstddef>
#include <type_traits>

typedef uint8_t byte;

#ifdef _LINUX // Fore matching Windows counterparts
	typedef int SOCKET;
#endif
#ifdef _WINDOWS // Fore matching Linux counterparts
	typedef uint32_t uint;
	typedef uint64_t ulong;
#endif

// template<class T> inline constexpr T operator~ (T a) { return (T)~(int)a; }
// template<class T> inline constexpr T operator| (T a, T b) { return (T)((int)a | (int)b); }
// template<class T> inline constexpr T operator& (T a, T b) { return (T)((int)a & (int)b); }
// template<class T> inline constexpr T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
// template<class T> inline constexpr T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
// template<class T> inline constexpr T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
// template<class T> inline constexpr T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

// Implicit cast for enum class using operator+
template <typename ENUMCLASS_T>
requires std::is_enum_v<ENUMCLASS_T>
constexpr auto operator+(ENUMCLASS_T e) noexcept {
	return static_cast<std::underlying_type_t<ENUMCLASS_T>>(e);
}
