#include <v4d.h>

using namespace v4d::networking;

OutgoingConnection::OutgoingConnection(v4d::io::SOCKET_TYPE type, v4d::crypto::RSA* serverPublicKey, int aesBits)
	: id(0), token(""), socket(type), rsa(serverPublicKey), aes(aesBits) {}

OutgoingConnection::OutgoingConnection(ulong id, std::string token, v4d::io::SOCKET_TYPE type, v4d::crypto::RSA* serverPublicKey, std::string aesHex)
	: id(id), token(token), socket(type), rsa(serverPublicKey), aes(aesHex) {}

OutgoingConnection::OutgoingConnection(OutgoingConnection& c)
	: id(c.id), token(c.token), socket(c.socket.IsTCP()?v4d::io::TCP:v4d::io::UDP), rsa(c.rsa), aes(c.aes.GetHexKey()) {}

OutgoingConnection::~OutgoingConnection(){
	Disconnect();
}

void OutgoingConnection::Disconnect() {
	socket.Disconnect();
	if (asyncRunThread) {
		if (asyncRunThread->joinable()) {
			asyncRunThread->join();
			delete asyncRunThread;
			asyncRunThread = nullptr;
		}
	}
}

void OutgoingConnection::SendHello(byte clientType) {
	socket << ZAP::HELLO << zapdata::ClientHello{GetAppName(), GetVersion(), clientType};
}

bool OutgoingConnection::Connect(std::string ip, uint16_t port, byte clientType) {
	Connect:
	socket.Connect(ip, port);
	SendHello(clientType);

	if (id) {
		socket << ZAP::TOKEN;
		LOG_VERBOSE("Connecting using TOKEN....")
		if (!TokenRequest()) {
			LOG_ERROR_VERBOSE("Token Connection failed, will try Auth connection...")
			id = 0;
			goto Connect;
		}
		return true;
	} else {
		v4d::data::Stream authData(rsa? rsa->GetMaxBlockSize() : 256);
		Authenticate(&authData);
		if (authData.GetWriteBufferSize() == 0) {
			LOG_VERBOSE("Connecting anonymously....")
			socket << ZAP::ANONYM;
			return AnonymousRequest();
		} else {
			LOG_VERBOSE("Connecting using AUTH....")
			socket << ZAP::AUTH;
			return AuthRequest(authData);
		}
	}

	return false;
}

bool OutgoingConnection::ConnectRunAsync(std::string ip, uint16_t port, byte clientType) {
	runAsync = true;
	return Connect(ip, port, clientType);
}

std::string OutgoingConnection::GetServerPublicKey(std::string ip, uint16_t port) {
	std::string publicKey{""};
	socket.Connect(ip, port);
	SendHello();
	LOG_VERBOSE("Getting server public key....")
	socket << ZAP::PUBKEY;
	socket.Flush();
	if (socket.Read<byte>() == ZAP::OK) {
		publicKey = socket.Read<std::string>();
	}
	socket.Disconnect();
	return publicKey;
}

bool OutgoingConnection::TokenRequest() {
	static std::atomic<ulong> requestIncrement = 0;
	socket << zapdata::ClientToken{id, ++requestIncrement, zapdata::EncryptedString{}.Encrypt(&aes, token)};
	if (socket.IsUDP()) return true;
	socket.Flush();
	if (socket.Poll(connectionTimeoutMilliseconds) <= 0) {
		Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
		return false;
	}
	switch (socket.Read<byte>()) {
		case ZAP::OK:
			return HandleConnection();
		break;
		case ZAP::DENY:
			auto[errCode, errMsg] = zapdata::Error::ReadFrom(socket);
			Error(errCode, "Error while trying to connect to server: " + errMsg);
			return false;
	}
	return false;
}

bool OutgoingConnection::AnonymousRequest() {
	socket.Flush();
	if (socket.Poll(connectionTimeoutMilliseconds) <= 0) {
		Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
		return false;
	}
	switch (socket.Read<byte>()) {
		case ZAP::OK:
			return HandleConnection();
		break;
		case ZAP::DENY:
			auto[errCode, errMsg] = zapdata::Error::ReadFrom(socket);
			Error(errCode, "Error while trying to connect to server: " + errMsg);
			return false;
	}
	return false;
}

bool OutgoingConnection::AuthRequest(v4d::data::Stream& authData) {
	authData << aes.GetHexKey();
	if (rsa) {
		socket.WriteEncryptedStream(rsa, authData);
	} else {
		socket.WriteStream(authData);
	}
	socket.Flush();
	if (socket.IsUDP()) {
		return HandleConnection();
	}
	if (socket.Poll(connectionTimeoutMilliseconds) <= 0) {
		Error(ZAP_CODES::SERVER_RESPONSE_TIMEOUT, ZAP_CODES::SERVER_RESPONSE_TIMEOUT_text);
		return false;
	}
	switch (socket.Read<byte>()) {
		case ZAP::OK:
			{
				if (rsa) {
					auto signature = socket.Read<std::vector, byte>();
					if (!rsa->Verify(authData.GetData(), signature)) {
						Error(ZAP_CODES::SERVER_SIGNATURE_FAILED, "Error while trying to connect to server. " + ZAP_CODES::SERVER_SIGNATURE_FAILED_text);
						return false;
					}
				}
				auto tokenAndId = socket.ReadEncryptedStream(&aes);
				tokenAndId >> token >> id;
				return HandleConnection();
			}
		break;
		case ZAP::DENY:
			auto[errCode, errMsg] = zapdata::Error::ReadFrom(socket);
			Error(errCode, "Error while trying to connect to server: " + errMsg);
			return false;
	}
	return false;
}

bool OutgoingConnection::HandleConnection() {
	if (runAsync) {
		asyncRunThread = new std::thread(&OutgoingConnection::Run, this, std::ref(socket));
	} else {
		Run(socket);
	}
	return true;
}

void OutgoingConnection::Error(int code, std::string message) const {
	LOG_ERROR("(" << code << ") " << message);
}
