#pragma once

#if _LINUX // Fore matching Windows counterparts
	typedef std::uint8_t byte;
	typedef int SOCKET;
#endif

typedef std::uint16_t ushort;
typedef std::uint32_t uint;
typedef std::uint64_t ulong;

typedef std::uint8_t index_255;
typedef std::uint16_t index_65k;
typedef std::uint32_t index_int;
typedef std::uint64_t index_long;

