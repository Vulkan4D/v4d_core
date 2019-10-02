#pragma once

#include <v4d.h>

namespace v4d::crypto {

	class V4DLIB RSA : public Crypto {
	private:
		void* rsaHandle;
		RSA(void*);

	public:
		std::string GetPrivateKeyPEM() const;
		std::string GetPublicKeyPEM() const;
		size_t GetSize() const;
		size_t GetBits() const;
		size_t GetMaxBlockSize() const;
		static RSA FromPrivateKeyPEM(const std::string&);
		static RSA FromPublicKeyPEM(const std::string&);
		
		RSA(int bits, long unsigned int exponent);
		~RSA();
		DELETE_COPY_MOVE_CONSTRUCTORS(RSA)

		using Crypto::Encrypt;
		using Crypto::Decrypt;

		std::vector<byte> Encrypt(const byte* data, size_t) override;
		std::vector<byte> Decrypt(const byte* data, size_t) override;

		std::vector<byte> Sign(const byte* data, size_t size);
		bool Verify(const byte* data, size_t size, const std::vector<byte>& signature);

		std::vector<byte> Sign(const std::vector<byte>& data);
		bool Verify(const std::vector<byte>& data, const std::vector<byte>& signature);
	};

}
