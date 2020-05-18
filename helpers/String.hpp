#pragma once
#include <v4d.h>

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
	
	};
	
}
