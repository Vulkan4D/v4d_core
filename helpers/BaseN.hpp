#pragma once
#include <v4d.h>

namespace v4d {
	static const std::string BASE36_UPPER_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const std::string BASE36_LOWER_CHARS = "0123456789abcdefghijklmnopqrstuvwxyz";
	static const std::string BASE26_UPPER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const std::string BASE26_LOWER_CHARS = "abcdefghijklmnopqrstuvwxyz";
	
	class BaseN {
	public:
		
		static std::string EncodeLong(uint64_t value, const std::string& BASECHARS = BASE36_UPPER_CHARS) {
			std::string str {""};
			uint64_t base = BASECHARS.length();
			while (value > 0) {
				uint64_t v = value % base;
				value -= v;
				value /= base;
				str += BASECHARS[v];
			}
			return str;
		}

		static uint64_t DecodeLong(std::string str, const std::string& BASECHARS = BASE36_UPPER_CHARS) {
			uint64_t value = 0;
			uint64_t base = BASECHARS.length();
			for (int i = str.length()-1; i >= 0; --i) {
				uint64_t pos = (uint64_t) BASECHARS.find(str[i]);
				if (pos == std::string::npos) return 0;
				if (double(value) * double(base) > double(std::numeric_limits<uint64_t>::max())) return 0;
				value *= base;
				if (double(value) + double(pos) > double(std::numeric_limits<uint64_t>::max())) return 0;
				value += pos;
			}
			return value;
		}

	};
}
