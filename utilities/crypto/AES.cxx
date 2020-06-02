#include <v4d.h>

namespace v4d::networking::ZAP::data {
	ZAPDATA( ClientTokenTest, 
		Int64_u increment;
		EncryptedString token;
	)
}

namespace v4d::tests {
	int AES() {
		int result = 0;

		{// Test1: Encrypt / Decrypt
			result = 100;
			v4d::data::Stream stream(1024);
			stream << 20;
			stream << true;
			stream << std::string("testing AES...");
			stream << 65;
			stream << 15.0;

			result += (int)stream.GetWriteBufferSize();

			v4d::crypto::AES aes(256);
			auto aesHex = aes.GetHexKey();
			v4d::crypto::AES aes2(aesHex);

			if (aesHex != aes2.GetHexKey()) {
				LOG_ERROR("ERROR: AES Test1 failed: converting to/from hex string")
				return -1;
			}

			auto encryptedData = aes.EncryptStream(stream);
			auto decryptedStream = aes.DecryptStream(encryptedData);

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

		{// Test2: Encrypt / Decrypt String
			v4d::crypto::AES aes(256);
			auto aesHex = aes.GetHexKey();
			v4d::crypto::AES aes2(aesHex);

			auto encryptedData = aes.EncryptString("Hello AES!");
			auto decryptedString = aes2.DecryptString(encryptedData);

			if (decryptedString != "Hello AES!") {
				LOG_ERROR("ERROR: AES Test2 failed")
				return 2;
			}
		}

		{// Test3: Encrypt / Decrypt Array of bytes
			result = 100;
			
			v4d::crypto::AES aes(256);

			byte bytes[6] = {3, 17, 50, 10, 12, 8};

			auto encryptedData = aes.Encrypt(bytes, 6);
			auto decryptedData = aes.Decrypt(encryptedData);

			for (byte b : decryptedData) result -= b;

			if (result != 0) LOG_ERROR("ERROR: AES Test3 failed")
		}

		{// Test4: Encrypt / Decrypt ZAP String
			v4d::crypto::AES aes(256);
			auto aesHex = aes.GetHexKey();
			v4d::crypto::AES aes2(aesHex);

			std::string token = "123456789";
			auto data = zapdata::ClientTokenTest{4, zapdata::EncryptedString{}.Encrypt(&aes, token)};
			std::string token2 = data.token.Decrypt(&aes2);

			if (strcmp(token.c_str(), token2.c_str()) != 0) LOG_ERROR("ERROR: AES Test4 failed")
		}


		return result;
	}
}
