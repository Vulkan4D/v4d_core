#pragma once

#if _LINUX
	typedef std::uint8_t byte;
#endif

#define BYTE_MAX_VALUE 255

typedef std::uint16_t ushort;
typedef std::uint32_t uint;
typedef std::uint64_t ulong;

typedef std::uint8_t index_255;
typedef std::uint16_t index_65k;
typedef std::uint32_t index_int;
typedef std::uint64_t index_long;

