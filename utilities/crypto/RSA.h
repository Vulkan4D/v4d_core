#pragma once

#include <v4d.h>

namespace v4d::crypto {

	class V4DLIB RSA {
	private:
		void* rsaHandle;
		RSA(void*);

	public:
		std::string GetPrivateKeyPEM() const;
		std::string GetPublicKeyPEM() const;
		inline size_t GetSize() const;
		inline size_t GetBits() const;
		static RSA FromPrivateKeyPEM(const std::string&);
		static RSA FromPublicKeyPEM(const std::string&);
		
		RSA(int bits, long unsigned int exponent);
		~RSA();
		DELETE_COPY_CONSTRUCTORS(RSA)

		std::vector<byte> Encrypt(const std::vector<byte>& data);
		std::vector<byte> Decrypt(const std::vector<byte>& encryptedData);
		std::vector<byte> Sign(const std::vector<byte>& data);
		bool Verify(const std::vector<byte>& data, const std::vector<byte>& signature);
	};

}
