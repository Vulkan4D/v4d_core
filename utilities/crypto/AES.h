#pragma once

#include <v4d.h>

namespace v4d::crypto {
	
	class V4DLIB AES : public Crypto {
	private:
		void* AesKeyHandle;
		void* AesKeyDecryptHandle;
		void InitAes();
	public:
		std::vector<byte> key;
		std::vector<byte> iv;
		
		AES(int keyBits);
		AES(const std::string& hexKey);
		AES(const std::vector<byte>& key);
		~AES();
		DELETE_COPY_CONSTRUCTORS(AES)

		using Crypto::Encrypt;
		using Crypto::Decrypt;

		std::string GetHexKey() const;

		std::vector<byte> Encrypt(const byte* data, size_t) override;
		std::vector<byte> Decrypt(const byte* data, size_t) override;
	};
}
