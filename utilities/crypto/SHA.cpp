#include <v4d.h>

#include <openssl/sha.h>

std::string v4d::crypto::SHA1(const byte* data, size_t size) {
	byte hash[SHA_DIGEST_LENGTH];
	char outputBuffer[SHA_DIGEST_LENGTH*2+1];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, data, size);
    SHA1_Final(hash, &sha1);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[SHA_DIGEST_LENGTH * 2] = 0;
	return std::string(outputBuffer);
}
std::string v4d::crypto::SHA1(const std::vector<byte>& data) {
	return SHA1(data.data(), data.size());
}
std::string v4d::crypto::SHA1(const std::string& str) {
	return SHA1((byte*)str.c_str(), str.length());
}

std::string v4d::crypto::SHA256(const byte* data, size_t size) {
	byte hash[SHA256_DIGEST_LENGTH];
	char outputBuffer[SHA256_DIGEST_LENGTH*2+1];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, size);
    SHA256_Final(hash, &sha256);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[SHA256_DIGEST_LENGTH * 2] = 0;
	return std::string(outputBuffer);
}
std::string v4d::crypto::SHA256(const std::vector<byte>& data) {
	return SHA256(data.data(), data.size());
}
std::string v4d::crypto::SHA256(const std::string& str) {
	return SHA256((byte*)str.c_str(), str.length());
}
