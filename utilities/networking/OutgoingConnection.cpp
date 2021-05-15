#include "OutgoingConnection.h"
#include "utilities/io/Logger.h"

using namespace v4d::networking;

OutgoingConnection::OutgoingConnection(v4d::io::SOCKET_TYPE type, std::shared_ptr<v4d::crypto::RSA> serverPublicKey, int aesBits)
	: id(-1), token(""), socket(nullptr), rsa(serverPublicKey && serverPublicKey->GetSize()? serverPublicKey : nullptr), aes(aesBits) {
		socket = std::make_shared<v4d::io::Socket>(type);
	}

OutgoingConnection::OutgoingConnection(int32_t id, std::string token, v4d::io::SOCKET_TYPE type, std::shared_ptr<v4d::crypto::RSA> serverPublicKey, std::string aesHex)
	: id(id), token(token), socket(nullptr), rsa(serverPublicKey && serverPublicKey->GetSize()? serverPublicKey : nullptr), aes(aesHex) {
		socket = std::make_shared<v4d::io::Socket>(type);
	}

OutgoingConnection::OutgoingConnection(OutgoingConnection* src)
: id(src->id), token(src->token), socket(std::make_shared<v4d::io::Socket>(src->socket->GetSocketType())), rsa(src->rsa), aes(src->aes.GetHexKey()) {
	socket->SetRemoteAddr(src->socket->GetRemoteAddr());
}

OutgoingConnection::OutgoingConnection(OutgoingConnection* src, v4d::io::SOCKET_TYPE type)
: id(src->id), token(src->token), socket(std::make_shared<v4d::io::Socket>(type)), rsa(src->rsa), aes(src->aes.GetHexKey()) {
	socket->SetRemoteAddr(src->socket->GetRemoteAddr());
}

OutgoingConnection::~OutgoingConnection(){
	Disconnect();
}

void OutgoingConnection::Disconnect() {
	socket->Disconnect();
	if (asyncRunThread) {
		if (asyncRunThread->joinable()) {
			asyncRunThread->join();
			delete asyncRunThread;
			asyncRunThread = nullptr;
		}
	}
}

void OutgoingConnection::SendHello(byte clientType) {
	*socket << ZAP::HELLO << zapdata::ClientHello{GetAppName(), GetVersion(), clientType};
}

bool OutgoingConnection::Connect(std::string ip, uint16_t port, byte clientType) {
	Connect:
	socket->Connect(ip, port);
	SendHello(clientType);

	if (id != -1) {
		*socket << ZAP::TOKEN;
		if (socket->IsTCP()) LOG_VERBOSE("Connecting using TOKEN....")
		if (!TokenRequest()) {
			if (socket->IsTCP()) { 
				LOG_ERROR_VERBOSE("Token Connection failed, will try Auth connection...")
				id = -1;
				goto Connect;
			} else {
				return false;
			}
		}
		return true;
	} else {
		v4d::data::Stream authData(rsa? rsa->GetMaxBlockSize() : 256);
		Authenticate(&authData);
		if (authData.GetWriteBufferSize() == 0) {
			LOG_VERBOSE("Connecting anonymously....")
			*socket << ZAP::ANONYM;
			return AnonymousRequest();
		} else {
			LOG_VERBOSE("Connecting using AUTH....")
			*socket << ZAP::AUTH;
			return AuthRequest(authData);
		}
	}

	return false;
}

bool OutgoingConnection::ConnectCommunicateAsync(std::string ip, uint16_t port, byte clientType) {
	runAsync = true;
	return Connect(ip, port, clientType);
}

std::string OutgoingConnection::GetServerPublicKey(std::string ip, uint16_t port) {
	std::string publicKey{""};
	socket->Connect(ip, port);
	SendHello();
	LOG_VERBOSE("Getting server public key....")
	*socket << ZAP::PUBKEY;
	socket->Flush();
	if (socket->Read<byte>() == ZAP::OK) {
		publicKey = socket->Read<std::string>();
	}
	socket->Disconnect();
	return publicKey;
}

bool OutgoingConnection::TokenRequest() {
	static std::atomic<uint64_t> requestIncrement = 0;
	socket->Write<int32_t>(id);
	v4d::data::Stream tokenToEncrypt(64);
	tokenToEncrypt << zapdata::ClientToken {++requestIncrement, token};
	socket->WriteEncryptedStream(&aes, tokenToEncrypt);
	if (socket->IsUDP()) {
		return HandleConnection();
	} else {
		socket->Flush();
		if (socket->Poll(connectionTimeoutMilliseconds) <= 0) {
			Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
			return false;
		}
		switch (socket->Read<byte>()) {
			case ZAP::OK:
				return HandleConnection();
			break;
			case ZAP::DENY:
				auto[errCode, errMsg] = zapdata::Error::ConstructFromStream(socket);
				Error(errCode, "Error while trying to connect to server: " + errMsg);
				return false;
		}
	}
	return false;
}

bool OutgoingConnection::AnonymousRequest() {
	if (socket->IsUDP()) {
		return HandleConnection();
	} else {
		socket->Flush();
		if (socket->Poll(connectionTimeoutMilliseconds) <= 0) {
			Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
			return false;
		}
		switch (socket->Read<byte>()) {
			case ZAP::OK:
				return HandleConnection();
			break;
			case ZAP::DENY:
				auto[errCode, errMsg] = zapdata::Error::ConstructFromStream(socket);
				Error(errCode, "Error while trying to connect to server: " + errMsg);
				return false;
		}
		LOG_ERROR("Unknown auth response from server")
	}
	return false;
}

bool OutgoingConnection::AuthRequest(v4d::data::Stream& authData) {
	authData << aes.GetHexKey();
	if (rsa) {
		socket->WriteEncryptedStream(rsa.get(), authData);
	} else {
		socket->WriteStream(authData);
	}
	socket->Flush();
	if (socket->IsUDP()) {
		return HandleConnection();
	}
	if (socket->Poll(connectionTimeoutMilliseconds) <= 0) {
		Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
		return false;
	}
	switch (socket->Read<byte>()) {
		case ZAP::OK:{
			if (rsa) {
				auto signature = socket->Read<std::vector, byte>();
				if (!rsa->Verify(authData.GetData(), signature)) {
					Error(ZAP_CODES::SERVER_SIGNATURE_FAILED, "Error while trying to connect to server. " + ZAP_CODES::SERVER_SIGNATURE_FAILED_text);
					return false;
				}
			}
			auto tokenAndId = socket->ReadEncryptedStream(&aes);
			tokenAndId >> token >> id;
			return HandleConnection();
		}
		break;
		case ZAP::DENY:{
			auto[errCode, errMsg] = zapdata::Error::ConstructFromStream(socket);
			Error(errCode, "Error while trying to connect to server: " + errMsg);
			return false;
		}
	}
	LOG_ERROR("Unknown auth response from server")
	return false;
}

bool OutgoingConnection::HandleConnection() {
	if (runAsync) {
		if (!asyncRunThread) {
			asyncRunThread = new std::thread(&OutgoingConnection::Communicate, this, socket);
		}
	} else {
		Communicate(socket);
	}
	return true;
}

void OutgoingConnection::Error(int code, std::string message) const {
	LOG_ERROR("(" << code << ") " << message);
}
