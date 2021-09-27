#pragma once

#include <string>

namespace v4d {
	// Maximum of 11 chars in 0123456789abcdefghijklmnopqrstuvwxyz_-. including space
	struct TextID {
		uint64_t numericValue;
		
		inline static const char* BASECHARS = BASE40_LOWER_CHARS; // or BASE64_WORD_DASH_CHARS with 10 chars?
		inline static constexpr int MAX_LENGTH = 11;
		
		TextID(uint64_t v = 0) : numericValue(v) {}
		TextID(const char* str) : numericValue(BaseN::TryEncodeStringToUInt64(str, BASECHARS)) {}
		TextID(const std::string_view& str) : numericValue(BaseN::TryEncodeStringToUInt64(str, BASECHARS)) {}
		
		TextID(const TextID& other) : numericValue(other.numericValue) {}
		TextID(TextID&& other) : numericValue(std::move(other.numericValue)) {}
		
		static TextID TryOrHash(const std::string_view& str) {
			try {
				return str;
			} catch (...) {
				return std::hash<std::string_view>()(str);
			}
		}
		
		TextID& operator=(uint64_t v) {
			numericValue = v;
			return *this;
		}
		TextID& operator=(const char* str) {
			numericValue = BaseN::TryEncodeStringToUInt64(str, BASECHARS);
			return *this;
		}
		TextID& operator=(const std::string_view& str) {
			numericValue = BaseN::TryEncodeStringToUInt64(str, BASECHARS);
			return *this;
		}
		TextID& operator=(const TextID& other) {
			numericValue = other.numericValue;
			return *this;
		}
		
		operator uint64_t () const {
			return numericValue;
		}
		operator std::string () const {
			return BaseN::DecodeStringFromUInt64(numericValue, BASECHARS);
		}
		
		explicit operator bool() const {
			return numericValue != 0;
		}
		
		bool operator==(const TextID& other) {
			return numericValue == other.numericValue;
		}
		bool operator!=(const TextID& other) {
			return numericValue != other.numericValue;
		}
		
	};
}

namespace std {
	template<>
	struct hash<v4d::TextID> {
		size_t operator()(const v4d::TextID& v) const {
			return hash<uint64_t>{}(v.numericValue);
		}
	};
}
