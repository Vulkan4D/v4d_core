#pragma once

#include <v4d.h>

namespace v4d {
	static const char* const base16Chars = "0123456789ABCDEF";
	class Base16 {
	public:

		INLINE static std::string Encode(std::vector<byte> data) {
			return Encode(data.data(), data.size());
		}

		static std::string Encode(const byte* data, size_t dataLen) {
			std::string ret;
			ret.reserve(dataLen * 2);
			for (size_t i = 0; i < dataLen; ++i) {
				const unsigned char b = data[i];
				ret.push_back(base16Chars[(size_t)(b >> 4)]);
				ret.push_back(base16Chars[(size_t)(b & 15)]);
			}
			return ret;
		}

		static std::vector<byte> Decode(const std::string& str) {
			size_t hexSize = str.length();
			std::vector<byte> ret;
			ret.reserve(hexSize / 2);
			for (size_t i = 0; i < hexSize; i+=2) {
				char a = str[i];
				const char* p = std::lower_bound(base16Chars, base16Chars + 16, a);
				if (*p != a) throw std::invalid_argument("not a hex digit");

				char b = str[i + 1];
				const char* q = std::lower_bound(base16Chars, base16Chars + 16, b);
				if (*q != b) throw std::invalid_argument("not a hex digit");

				ret.push_back((byte) ((byte)((p - base16Chars) << 4) | (byte)(q - base16Chars)));
			}
			return ret;
		}
	};
}
