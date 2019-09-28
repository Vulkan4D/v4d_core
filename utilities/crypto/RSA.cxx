#include <v4d.h>

namespace v4d::tests {
	int RSA() {
		int result = 0;

		// Generate a new RSA KeyPair and separate it into public and private keys
		auto rsa = v4d::crypto::RSA(2048, 3);
		std::string privateKeyPEM = rsa.GetPrivateKeyPEM();
		std::string publicKeyPEM = rsa.GetPublicKeyPEM();
		auto privateKey = v4d::crypto::RSA::FromPrivateKeyPEM(privateKeyPEM);
		auto publicKey = v4d::crypto::RSA::FromPublicKeyPEM(publicKeyPEM);

		{// Test1: Encrypt / Decrypt
			result = 100;
			v4d::data::Stream stream(1024);
			stream << 30;
			stream << true;
			stream << std::string("testing RSA Encryption...");
			stream << 55;
			stream << 15.0;

			auto encryptedData = publicKey.EncryptStream(stream);
			auto decryptedStream = privateKey.DecryptStream(encryptedData);

			int a = 0;
			bool b = false;
			std::string c = "";
			int d = 0;
			double e = 0.0;

			decryptedStream >> a >> b >> c >> d >> e;

			if (b && c == "testing RSA Encryption...") {
				result -= a;
				result -= d;
				result -= (int)e;
			}

			if (result != 0) {
				LOG_ERROR("ERROR: RSA Test1 failed")
				return 1;
			}
		}

		{// Test2: Encrypt / Decrypt String

			auto encryptedData = rsa.EncryptString("Hello RSA!");
			auto decryptedString = rsa.DecryptString(encryptedData);

			if (decryptedString != "Hello RSA!") {
				LOG_ERROR("ERROR: RSA Test2 failed")
				return 2;
			}
		}

		{// Test3: Encrypt / Decrypt Array of bytes
			result = 100;
			
			byte bytes[6] = {3, 17, 50, 10, 12, 8};

			auto encryptedData = rsa.Encrypt(bytes, 6);
			auto decryptedData = rsa.Decrypt(encryptedData);

			for (byte b : decryptedData) result -= b;

			if (result != 0) {
				LOG_ERROR("ERROR: RSA Test3 failed")
				return 3;
			}
		}

		{// Test4: Sign / Verify
			result = 100;
			v4d::data::Stream stream(1024);
			stream << 15;
			stream << true;
			stream << std::string("testing RSA Signature...");
			stream << 55;
			stream << 30.0;

			auto rsa2 = v4d::crypto::RSA(2048, 3);
			
			auto signature = rsa.Sign(stream._GetWriteBuffer_());
			auto signature2 = rsa2.Sign(stream._GetWriteBuffer_());

			if (rsa.Verify(stream._GetWriteBuffer_(), signature)) {
				result -= 90;
			}
			if (rsa2.Verify(stream._GetWriteBuffer_(), signature2)) {
				result -= 10;
			}
			if (rsa2.Verify(stream._GetWriteBuffer_(), signature)) {
				result -= 20;
			}
			if (rsa.Verify(stream._GetWriteBuffer_(), signature2)) {
				result -= 30;
			}

			if (result != 0) {
				LOG_ERROR("ERROR: RSA Test4 failed")
				return 4;
			}
		}

		return result;
	}
}
