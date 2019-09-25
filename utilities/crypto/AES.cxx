#include <v4d.h>

namespace v4d::tests {
	int AES() {
		int result = 0;

		{// Test1: Encrypt / Decrypt
			result = 100;
			v4d::Stream stream(1024);
			stream << 20;
			stream << true;
			stream << std::string("testing AES...");
			stream << 65;
			stream << 15.0;

			result += (int)stream._GetWriteBuffer_().size();

			v4d::crypto::AES aes(256);
			auto aesHex = aes.GetHexKey();
			v4d::crypto::AES aes2(aesHex);

			if (aesHex != aes2.GetHexKey()) {
				LOG_ERROR("ERROR: AES Test1 failed: converting to/from hex string")
				return -1;
			}

			auto encryptedData = aes.Encrypt(stream._GetWriteBuffer_());
			v4d::ReadOnlyStream decryptedStream(aes.Decrypt(encryptedData));

			result -= (int)decryptedStream.GetDataBufferRemaining();

			int a = 0;
			bool b = false;
			std::string c = "";
			int d = 0;
			double e = 0.0;

			decryptedStream >> a >> b >> c >> d >> e;

			if (b && c == "testing AES...") {
				result -= a;
				result -= d;
				result -= (int)e;
			}

			if (result != 0) LOG_ERROR("ERROR: AES Test1 failed")
		}

		return result;
	}
}
