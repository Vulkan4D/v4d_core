#pragma once

#include <v4d.h>
#include <vector>
#include <string>

namespace v4d::data {
	class Stream;
	class ReadOnlyStream;
}

namespace v4d::crypto {
	
	class V4DLIB Crypto {
	public:
		Crypto();
		virtual ~Crypto();
		DELETE_COPY_MOVE_CONSTRUCTORS(Crypto)

		virtual std::vector<byte> Encrypt(const byte* data, size_t) = 0;
		virtual std::vector<byte> Decrypt(const byte* data, size_t) = 0;

		std::vector<byte> Encrypt(const std::vector<byte>& data);
		std::vector<byte> Decrypt(const std::vector<byte>& encryptedData);

		std::vector<byte> EncryptString(const std::string&);
		std::string DecryptString(const std::vector<byte>& encryptedData);

		std::vector<byte> EncryptStream(v4d::data::Stream& stream);
		v4d::data::ReadOnlyStream DecryptStream(const std::vector<byte>& encryptedData);
	};
}
