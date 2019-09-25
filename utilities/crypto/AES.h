#pragma once

#include <v4d.h>

namespace v4d::crypto {
	
	class V4DLIB AES {
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
		std::string GetHexKey() const;
		std::vector<byte> Encrypt(std::vector<byte> copiedData);
		std::vector<byte> Decrypt(const std::vector<byte>& encryptedData);
	};
}
