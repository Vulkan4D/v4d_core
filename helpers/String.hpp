#pragma once

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

	};
	
}
