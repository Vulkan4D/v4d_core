#pragma once

#include <string>

namespace v4d {
	// for maximum string length of 13, padding is first character (space)
	static const char BASE26_UPPER_CHARS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const char BASE26_LOWER_CHARS[] = " abcdefghijklmnopqrstuvwxyz";
	
	// for maximum string length of 12, padding is first character (space)
	static const char BASE36_UPPER_CHARS[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const char BASE36_LOWER_CHARS[] = " 0123456789abcdefghijklmnopqrstuvwxyz";
	
	// for maximum string length of 11, padding is first character (tab)
	static const char BASE40_UPPER_CHARS[] = "\t0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_-. ";
	static const char BASE40_LOWER_CHARS[] = "\t0123456789abcdefghijklmnopqrstuvwxyz_-. ";
	
	// for maximum string length of 10, padding is first character (space)
	static const char BASE64_WORD_DOT_CHARS[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.";
	static const char BASE64_WORD_DASH_CHARS[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";
	
	class BaseN {
	public:
		
		static std::string DecodeStringFromUInt64(uint64_t value, const std::string_view& BASECHARS = BASE36_UPPER_CHARS) noexcept {
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

		static uint64_t EncodeStringToUInt64(std::string_view str, const std::string_view& BASECHARS = BASE36_UPPER_CHARS) noexcept {
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

		static uint64_t TryEncodeStringToUInt64(std::string_view str, const std::string_view& BASECHARS = BASE36_UPPER_CHARS) {
			uint64_t value = 0;
			uint64_t base = BASECHARS.length();
			for (int i = str.length()-1; i >= 0; --i) {
				uint64_t pos = (uint64_t) BASECHARS.find(str[i]);
				if (pos == std::string::npos) throw std::runtime_error("TextID: Invalid string '" + std::string(str) + "'");
				if (double(value) * double(base) > double(std::numeric_limits<uint64_t>::max())) throw std::runtime_error("TextID: Invalid string '" + std::string(str) + "'");
				value *= base;
				if (double(value) + double(pos) > double(std::numeric_limits<uint64_t>::max())) throw std::runtime_error("TextID: Invalid string '" + std::string(str) + "'");
				value += pos;
			}
			return value;
		}

	};
}
