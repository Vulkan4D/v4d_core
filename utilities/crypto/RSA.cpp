// Pre-compiled common header
#include <common/pch.hh>

// V4D Core Header
#include <v4d.h>

#define OPENSSL_RSA_ERR(action) \
	ERR_load_CRYPTO_strings();\
	char errorMessage[200];\
	ERR_error_string_n(ERR_get_error(), errorMessage, 200);\
	LOG_ERROR("RSA " << action << " Error: " << errorMessage)


v4d::crypto::RSA::RSA(int bits, long unsigned int exponent) {
	rsaHandle = RSA_new();
	BIGNUM* bne = BN_new();
	BN_set_word(bne, exponent);
	RSA_generate_key_ex((::RSA*)rsaHandle, bits, bne, NULL);
	BN_free(bne);
}

v4d::crypto::RSA::RSA(void* rsaHandle) {
	this->rsaHandle = (::RSA*)rsaHandle;
}

v4d::crypto::RSA::~RSA() {
	RSA_free((::RSA*)rsaHandle);
}

size_t v4d::crypto::RSA::GetSize() const {
	return (size_t)RSA_size((::RSA*)rsaHandle);
}
size_t v4d::crypto::RSA::GetBits() const {
	return (size_t)RSA_bits((::RSA*)rsaHandle);
}

size_t v4d::crypto::RSA::GetMaxBlockSize() const {
	return GetSize() - RSA_PKCS1_PADDING_SIZE;
}

std::string v4d::crypto::RSA::GetPrivateKeyPEM() const {
	BIO* bio = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bio, (::RSA*)rsaHandle, NULL, NULL, 0, NULL, NULL);
	size_t size = (size_t)BIO_pending(bio);
	char str[size];
	BIO_read(bio, str, (int)size);
	BIO_free(bio);
	return std::string(str, size);
}
std::string v4d::crypto::RSA::GetPublicKeyPEM() const {
	BIO* bio = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bio, (::RSA*)rsaHandle);
	size_t size = (size_t)BIO_pending(bio);
	char str[size];
	BIO_read(bio, str, (int)size);
	BIO_free(bio);
	return std::string(str, size);
}

v4d::crypto::RSA v4d::crypto::RSA::FromPrivateKeyPEM(const std::string& pem) {
	::RSA* rsa = RSA_new();
	BIO* bio = BIO_new(BIO_s_mem());
	BIO_write(bio, pem.c_str(), (int)pem.size());
	PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL);
	BIO_free(bio);
	return v4d::crypto::RSA{rsa};
}

v4d::crypto::RSA v4d::crypto::RSA::FromPublicKeyPEM(const std::string& pem) {
	::RSA* rsa = RSA_new();
	BIO* bio = BIO_new(BIO_s_mem());
	BIO_write(bio, pem.c_str(), (int)pem.size());
	PEM_read_bio_RSAPublicKey(bio, &rsa, NULL, NULL);
	BIO_free(bio);
	return v4d::crypto::RSA{rsa};
}

std::vector<byte> v4d::crypto::RSA::Encrypt(const byte* data, size_t size) {
	std::vector<byte> encryptedData(GetSize());
	if (size > GetMaxBlockSize()) {
		LOG_ERROR("RSA Encrypt Error: Max block size exceeded")
		encryptedData.clear();
		return encryptedData;
	}
	int res = RSA_public_encrypt((int)size, data, encryptedData.data(), (::RSA*)rsaHandle, RSA_PKCS1_PADDING);
	if (res != (int)GetSize())  {
		OPENSSL_RSA_ERR("Encrypt")
		encryptedData.clear();
		return encryptedData;
	}
	return encryptedData;
}

std::vector<byte> v4d::crypto::RSA::Decrypt(const byte* encryptedData, size_t size) {
	if (size != GetSize()) {
		LOG_ERROR("RSA Decrypt Error: invalid input data (block size does not match)")
		return std::vector<byte>{};
	}
	std::vector<byte> data(GetSize());
	int decryptedSize = RSA_private_decrypt((int)size, encryptedData, data.data(), (::RSA*)rsaHandle, RSA_PKCS1_PADDING);
	if (decryptedSize == -1) {
		OPENSSL_RSA_ERR("Decrypt")
		data.clear();
		return data;
	}
	data.resize((size_t)decryptedSize);
	return data;
}

std::vector<byte> v4d::crypto::RSA::Sign(const byte* data, size_t size) {
	std::vector<byte> signature(GetSize());
	unsigned int s;
	if (RSA_sign(NID_sha1, data, (unsigned int)size, signature.data(), &s, (::RSA*)rsaHandle) == 0) {
		OPENSSL_RSA_ERR("Sign")
		signature.clear();
		return signature;
	}
	return signature;
}

bool v4d::crypto::RSA::Verify(const byte* data, size_t size, const std::vector<byte>& signature) {
	return RSA_verify(NID_sha1, data, (unsigned int)size, signature.data(), (unsigned int)signature.size(), (::RSA*)rsaHandle) == 1;
}

std::vector<byte> v4d::crypto::RSA::Sign(const std::vector<byte>& data) {
	return Sign(data.data(), data.size());
}

bool v4d::crypto::RSA::Verify(const std::vector<byte>& data, const std::vector<byte>& signature) {
	return Verify(data.data(), data.size(), signature);
}
