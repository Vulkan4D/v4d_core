#include <v4d.h>

v4d::crypto::Crypto::Crypto() {}
v4d::crypto::Crypto::~Crypto() {}

std::vector<byte> v4d::crypto::Crypto::Encrypt(const std::vector<byte>& data) {
	return Encrypt(data.data(), data.size());
}

std::vector<byte> v4d::crypto::Crypto::Decrypt(const std::vector<byte>& encryptedData) {
	return Decrypt(encryptedData.data(), encryptedData.size());
}

std::vector<byte> v4d::crypto::Crypto::EncryptString(const std::string& str) {
	return Encrypt((byte*)str.c_str(), str.size());
}

std::string v4d::crypto::Crypto::DecryptString(const std::vector<byte>& encryptedData) {
	std::string str;
	auto decryptedData = Decrypt(encryptedData);
	str.insert(0, (char*)decryptedData.data(), decryptedData.size());
	return str;
}

std::vector<byte> v4d::crypto::Crypto::EncryptStream(v4d::data::Stream& stream) {
	return Encrypt(stream._GetWriteBuffer_());
}

v4d::data::ReadOnlyStream v4d::crypto::Crypto::DecryptStream(const std::vector<byte>& encryptedData) {
	return v4d::data::ReadOnlyStream(Decrypt(encryptedData));
}
