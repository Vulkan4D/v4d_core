#pragma once

#include <v4d.h>
#include <vector>

namespace v4d::crypto {
	class V4DLIB Random {
	public:
		template<typename T>
		static void Generate(T& value) {
			std::vector<byte> bytes(sizeof(T));
			Generate(bytes);
			memcpy(&value, bytes.data(), bytes.size());
		}
		static void Generate(std::vector<byte>& bytes);
	};
}
