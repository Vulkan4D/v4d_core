#include <v4d.h>

#include <openssl/rand.h>
#include <openssl/aes.h>

#define IV_SIZE AES_BLOCK_SIZE


v4d::crypto::AES::AES(int keyBits) {
	key.resize((size_t)keyBits/8);
	RAND_bytes(key.data(), keyBits/8);
	InitAes();
}

v4d::crypto::AES::AES(const std::vector<byte>& key) {
	this->key = key;
	InitAes();
}

void v4d::crypto::AES::InitAes() {
	// Generate new IV
	iv.resize((size_t)IV_SIZE);
	RAND_bytes(iv.data(), IV_SIZE);
	// Init handles
	AesKeyHandle = (void*) new AES_KEY();
	AES_set_encrypt_key(key.data(), (int)key.size()*8, (AES_KEY*)AesKeyHandle);
	AesKeyDecryptHandle = (void*) new AES_KEY();
	AES_set_decrypt_key(key.data(), (int)key.size()*8, (AES_KEY*)AesKeyDecryptHandle);
}

v4d::crypto::AES::~AES() {
	delete (AES_KEY*)AesKeyHandle;
	delete (AES_KEY*)AesKeyDecryptHandle;
}

std::vector<byte> v4d::crypto::AES::Encrypt(std::vector<byte> data) {
	// Resize data
	size_t actualDataSize = data.size();
	size_t paddingSize = AES_BLOCK_SIZE - ((data.size()+sizeof(size_t)) % AES_BLOCK_SIZE);
	data.resize(data.size() + paddingSize + sizeof(size_t));
	std::vector<byte> encryptedData(data.size() + IV_SIZE);
	// Add zeroes as padding
	memset(data.data() + (long)actualDataSize, 0, paddingSize);
	// Add actual data size after padding
	memcpy(data.data() + (long)actualDataSize + paddingSize, &actualDataSize, sizeof(size_t));
	// Add IV at the end of future encrypted data (iv stays unencrypted)
	memcpy(encryptedData.data() + (long)data.size(), iv.data(), IV_SIZE);
	// Encrypt
	AES_cbc_encrypt(data.data(), encryptedData.data(), data.size(), (AES_KEY*)AesKeyHandle, iv.data()/* IV will be modified here */, AES_ENCRYPT);
	return encryptedData;
}

std::vector<byte> v4d::crypto::AES::Decrypt(const std::vector<byte>& encryptedData) {
	// Prepare decrypted data vector
	std::vector<byte> decryptedData(encryptedData.size() - IV_SIZE);
	// Read IV from encrypted data
	byte _iv[IV_SIZE];
	memcpy(_iv, encryptedData.data() + (long)decryptedData.size(), IV_SIZE);
	// Decrypt
	AES_cbc_encrypt(encryptedData.data(), decryptedData.data(), encryptedData.size() - IV_SIZE, (const AES_KEY*) AesKeyDecryptHandle, _iv, AES_DECRYPT);
	// Read size from decrypted data and resize final decrypted data
	size_t actualDataSize;
	memcpy(&actualDataSize, decryptedData.data() + (long)decryptedData.size() - sizeof(size_t), sizeof(size_t));
	decryptedData.resize(actualDataSize);
	return decryptedData;
}

// Hex Key format : base16Hex( keySizeByte + key )
std::string v4d::crypto::AES::GetHexKey() const {
	std::vector<byte> sizeAndKey;
	sizeAndKey.resize(1 + key.size());
	sizeAndKey[0] = (byte)key.size();
	memcpy(sizeAndKey.data() + 1, key.data(), key.size());
	return v4d::Base16::Encode(sizeAndKey);
}
v4d::crypto::AES::AES(const std::string& hexKey) {
	auto sizeAndKey = v4d::Base16::Decode(hexKey);
	if (sizeAndKey.size() == 0) return; // Fix for compiler complaining about potential null pointer dereference...
	size_t keySize = sizeAndKey[0];
	key.resize(keySize);
	memcpy(key.data(), sizeAndKey.data() + 1, keySize);
	InitAes();
}
