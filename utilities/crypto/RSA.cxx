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
			v4d::Stream stream(1024);
			stream << 30;
			stream << true;
			stream << std::string("testing RSA Encryption...");
			stream << 55;
			stream << 15.0;

			auto encryptedData = publicKey.Encrypt(stream._GetWriteBuffer_());
			v4d::ReadOnlyStream decryptedStream(privateKey.Decrypt(encryptedData));

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

			if (result != 0) LOG_ERROR("ERROR: RSA Test1 failed")
		}

		{// Test2: Sign / Verify
			result = 100;
			v4d::Stream stream(1024);
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

			if (result != 0) LOG_ERROR("ERROR: RSA Test2 failed")
		}

		return result;
	}
}
