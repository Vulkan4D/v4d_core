#pragma once

#include <v4d.h>
#include <string>
#include <vector>

namespace v4d::crypto {
	V4DLIB std::string SHA1(const byte* data, size_t);
	V4DLIB std::string SHA1(const std::vector<byte>&);
	V4DLIB std::string SHA1(const std::string&);

	V4DLIB std::string SHA256(const byte* data, size_t);
	V4DLIB std::string SHA256(const std::vector<byte>&);
	V4DLIB std::string SHA256(const std::string&);

	V4DLIB std::string SHA512(const byte* data, size_t);
	V4DLIB std::string SHA512(const std::vector<byte>&);
	V4DLIB std::string SHA512(const std::string&);
}
