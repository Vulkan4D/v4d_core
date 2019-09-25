#pragma once

#include <v4d.h>

namespace v4d {
	const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	class Base64 {
	public:

		INLINE static std::string Encode(std::vector<byte> data) {
			return Encode(data.data(), data.size());
		}

		static std::string Encode(const byte* data, size_t dataLen) {
			std::string ret;

			int chrArr3[3], chrArr4[4];
			int i = 0, j = 0;

			while (dataLen--) {
				chrArr3[i++] = *(data++);
				if (i == 3) {
					chrArr4[0] = (chrArr3[0] & 0xfc) >> 2;
					chrArr4[1] = ((chrArr3[0] & 0x03) << 4) + ((chrArr3[1] & 0xf0) >> 4);
					chrArr4[2] = ((chrArr3[1] & 0x0f) << 2) + ((chrArr3[2] & 0xc0) >> 6);
					chrArr4[3] = chrArr3[2] & 0x3f;

					for(i = 0; i < 4; i++) ret += base64Chars[(size_t)chrArr4[i]];
					i = 0;
				}
			}

			if (i > 0) {
				for(j = i; j < 3; j++) chrArr3[j] = '\0';

				chrArr4[0] = (chrArr3[0] & 0xfc) >> 2;
				chrArr4[1] = ((chrArr3[0] & 0x03) << 4) + ((chrArr3[1] & 0xf0) >> 4);
				chrArr4[2] = ((chrArr3[1] & 0x0f) << 2) + ((chrArr3[2] & 0xc0) >> 6);
				chrArr4[3] = chrArr3[2] & 0x3f;

				for (j = 0; j < i+1; j++) ret += base64Chars[(size_t)chrArr4[j]];
				while (i++ < 3) ret += '=';
			}
			return ret;
		}

		static std::vector<byte> Decode(const std::string& str) {
			std::vector<byte> ret;

			int chrArr4[4], chrArr3[3];
			int i = 0, j = 0;
			size_t index = 0;

			size_t strLen = str.size();
			ret.reserve(strLen);

			while (strLen-- > 0 && str[index] != '=' && (isalnum(str[index]) || str[index] == '+' || str[index] == '/')) {
				chrArr4[i++] = str[index++];
				if (i == 4) {
					for (i = 0; i < 4; i++) chrArr4[i] = (int)base64Chars.find((char)chrArr4[i]);

					chrArr3[0] = (chrArr4[0] << 2) + ((chrArr4[1] & 0x30) >> 4);
					chrArr3[1] = ((chrArr4[1] & 0xf) << 4) + ((chrArr4[2] & 0x3c) >> 2);
					chrArr3[2] = ((chrArr4[2] & 0x3) << 6) + chrArr4[3];

					for (i = 0; i < 3; i++) ret.insert(ret.end(), (byte)chrArr3[i]);
					i = 0;
				}
			}

			if (i > 0) {
				for (j = i; j < 4; j++) chrArr4[j] = 0;
				for (j = 0; j < 4; j++) chrArr4[j] = (int)base64Chars.find((char)chrArr4[j]);

				chrArr3[0] = (chrArr4[0] << 2) + ((chrArr4[1] & 0x30) >> 4);
				chrArr3[1] = ((chrArr4[1] & 0xf) << 4) + ((chrArr4[2] & 0x3c) >> 2);
				chrArr3[2] = ((chrArr4[2] & 0x3) << 6) + chrArr4[3];

				for (j = 0; j < i-1; j++) ret.insert(ret.end(), (byte)chrArr3[j]);
			}

			return ret;
		}
	};
}
