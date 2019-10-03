#pragma once

#ifdef _LINUX // Fore matching Windows counterparts
typedef std::uint8_t byte;
typedef int SOCKET;
#endif

typedef std::uint16_t ushort;
typedef std::uint32_t uint;
typedef std::uint64_t ulong;

typedef size_t			index_t;
typedef std::uint8_t	index_255;
typedef std::uint16_t	index_65k;
typedef std::uint32_t	index_int;
typedef std::uint64_t	index_long;


/////////////////////////////////////////////////////////////////////////
// Compile-time Reflection for Looping through struct members (Maximum of 24 members for now)

template <size_t N>
using refl_members_count = std::integral_constant<size_t, N>;

struct __refl_any {
	template <typename T, typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>>
	operator T &&() const;
	template <typename T, typename = std::enable_if_t<std::is_copy_constructible<T>::value>>
	operator T&() const;
};

template <size_t N = 0>
static constexpr const __refl_any __refl_any_constructible{};

template <typename T, size_t... I>
inline constexpr auto __is_constructible_with_braces(std::index_sequence<I...>, T*) -> decltype(T{__refl_any_constructible<I>...}, std::true_type{}) {
	return {};
}

template <size_t... I>
inline constexpr std::false_type __is_constructible_with_braces(std::index_sequence<I...>, ...) {
	return {};
}

template <typename T, size_t N>
constexpr auto is_constructible_with_braces() -> decltype(__is_constructible_with_braces(std::make_index_sequence<N>{}, static_cast<T*>(nullptr))) {
	return {};
}

template <typename T, typename U>
struct __is_constructible_parenthesis;

template <typename T, size_t... I>
struct __is_constructible_parenthesis<T, std::index_sequence<I...>> : std::is_constructible<T, decltype(__refl_any_constructible<I>)...> {};

template <typename T, size_t N>
constexpr auto is_constructible_parenthesis() -> __is_constructible_parenthesis<T, std::make_index_sequence<N>> {
	return {};
}



#define __DEF_REFL_MEMBER_COUNT(count) \
template <typename T, typename = std::enable_if_t	<is_constructible_with_braces<T, count>() && \
													!is_constructible_with_braces<T, count+1>() && \
													!is_constructible_parenthesis<T, count>()>> \
constexpr refl_members_count<count> __ReflMemberCount(T&) { \
	return {};\
}

__DEF_REFL_MEMBER_COUNT(1)
__DEF_REFL_MEMBER_COUNT(2)
__DEF_REFL_MEMBER_COUNT(3)
__DEF_REFL_MEMBER_COUNT(4)
__DEF_REFL_MEMBER_COUNT(5)
__DEF_REFL_MEMBER_COUNT(6)
__DEF_REFL_MEMBER_COUNT(7)
__DEF_REFL_MEMBER_COUNT(8)
__DEF_REFL_MEMBER_COUNT(9)
__DEF_REFL_MEMBER_COUNT(10)
__DEF_REFL_MEMBER_COUNT(11)
__DEF_REFL_MEMBER_COUNT(12)
__DEF_REFL_MEMBER_COUNT(13)
__DEF_REFL_MEMBER_COUNT(14)
__DEF_REFL_MEMBER_COUNT(15)
__DEF_REFL_MEMBER_COUNT(16)
__DEF_REFL_MEMBER_COUNT(17)
__DEF_REFL_MEMBER_COUNT(18)
__DEF_REFL_MEMBER_COUNT(19)
__DEF_REFL_MEMBER_COUNT(20)
__DEF_REFL_MEMBER_COUNT(21)
__DEF_REFL_MEMBER_COUNT(22)
__DEF_REFL_MEMBER_COUNT(23)
__DEF_REFL_MEMBER_COUNT(24)



template <typename T> \
inline auto ReflMemberGroup(T& t, refl_members_count<0>) { \
	return std::tie(); \
}
#define __DEF_REFL_MEMBER_GROUP(n,...) \
template <typename T> \
inline auto ReflMemberGroup(T& t, refl_members_count<n>) { \
auto& [__VA_ARGS__] = t; \
	return std::tie(__VA_ARGS__); \
}
__DEF_REFL_MEMBER_GROUP(1,_0)
__DEF_REFL_MEMBER_GROUP(2,_1,_0)
__DEF_REFL_MEMBER_GROUP(3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
__DEF_REFL_MEMBER_GROUP(24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)
template <typename T>
inline auto ReflMemberGroup(T& t) {
	return ReflMemberGroup(t, __ReflMemberCount(t));
}


///////////////////////////////////////////////////////////////////
// Printable Struct/Class

template <typename T>
class ReflStructHasPrintMethod {
	typedef char yes[1]; typedef char no[2];
	template <typename classType> static yes& test( decltype(&classType::Print) ) ;
	template <typename classType> static no& test(...);
public: enum { value = (sizeof(test<T>(0)) == sizeof(yes)) };
};
template <typename T, typename Func>
inline void __ForEachPrintableMember(T& t, Func&& func) {
	if constexpr (ReflStructHasPrintMethod<T>::value) {
		func(t.Print());
	} else std::apply([&](auto&&... args) { (func(args), ...); }, ReflMemberGroup(t));
}
template <typename T, typename Func>
inline void __ForEachPrintableMember(Func&& func) {
	T t{}; __ForEachPrintableMember<T>(t, std::forward<Func>(func));
}

#define PRINTABLE(structName) \
friend std::ostream& operator<<(std::ostream& stream, structName const& obj) { \
	bool first = true; \
	stream << #structName << "{"; \
	__ForEachPrintableMember(obj, [&](auto&& memberValue) { \
		if (first) { \
			first = false; \
		} else { \
			stream << ", "; \
		} \
		stream << memberValue; \
	}); \
	return stream << "}"; \
}


///////////////////////////////////////////////////////////////////
