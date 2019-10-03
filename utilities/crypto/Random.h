#pragma once

#include <v4d.h>

namespace v4d::crypto {
	class V4DLIB Random {
	public:
		static void Generate(std::vector<byte>& bytes);
	};
}
