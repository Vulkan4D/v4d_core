#pragma once

#include <string>
#include <algorithm>
#include <cassert>
#include <cstring>

#define TRIM_CHARS " \f\n\r\t\v"

namespace v4d {

	struct String {

		static void TrimRight(std::string& s, const std::string& trimChars = TRIM_CHARS ) {
			s.erase( s.find_last_not_of( trimChars ) + 1 );
		}
		
		static void TrimLeft(std::string& s,  const std::string& trimChars = TRIM_CHARS ) {
			s.erase( 0, s.find_first_not_of( trimChars ) );
		}
		
		static void Trim(std::string& s, const std::string& trimChars = TRIM_CHARS ) {
			s.erase( s.find_last_not_of( trimChars ) + 1 ).erase( 0, s.erase( s.find_last_not_of( trimChars ) + 1 ).find_first_not_of( trimChars ) );
		}

		static void ToLowerCase(std::string& s) {
			std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
		}
		
		static void ToUpperCase(std::string& s) {
			std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
		}
		
		static bool Replace(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos = str.find(from);
			if (start_pos == std::string::npos) return false;
			str.replace(start_pos, from.length(), to);
			return true;
		}
		
		template <typename T>
		static std::string Hex(T w, size_t hex_len = sizeof(T)<<1) {
			static const char* digits = "0123456789ABCDEF";
			std::string rc(hex_len, '0');
			for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
				rc[i] = digits[(w>>j) & 0x0f];
			return rc;
		}
	
		static consteval uint16_t Interpret2CharsAsUInt16(const char* str) {
			assert(strlen(str) == 2);
			return uint64_t(str[0])
				| (uint64_t(str[1]) << 8)
			;
		}

		static consteval uint32_t Interpret4CharsAsUInt32(const char* str) {
			assert(strlen(str) == 4);
			return uint64_t(str[0])
				| (uint64_t(str[1]) << 8)
				| (uint64_t(str[2]) << 16)
				| (uint64_t(str[3]) << 24)
			;
		}

		static consteval uint64_t Interpret8CharsAsUInt64(const char* str) {
			assert(strlen(str) == 8);
			return uint64_t(str[0])
				| (uint64_t(str[1]) << 8)
				| (uint64_t(str[2]) << 16)
				| (uint64_t(str[3]) << 24)
				| (uint64_t(str[4]) << 32)
				| (uint64_t(str[5]) << 40)
				| (uint64_t(str[6]) << 48)
				| (uint64_t(str[7]) << 56)
			;
		}

	};
	
}
