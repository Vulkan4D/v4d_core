#include <v4d.h>

using namespace v4d::networking;

bool OutgoingConnection::Connect(std::string ip, uint16_t port, byte clientType) {
	Connect:
	socket.Connect(ip, port);

	// Hello + AppName + Version + ClientType
	socket << ZAP::HELLO << GetAppName() << GetVersion() << clientType;

	if (id) {
		socket << ZAP::TOKEN;
		if (!TokenRequest()) {
			LOG_ERROR("Token Connection failed, will try Auth connection...")
			id = 0;
			goto Connect;
		}
	} else {
		v4d::data::Stream authData(rsa? rsa->GetMaxBlockSize() : 256);
		Authenticate(&authData);
		if (authData.GetWriteBufferSize() == 0) {
			// LOG("Connecting anonymously....")
			socket << ZAP::ANONYM;
			return AnonymousRequest();
		} else {
			// LOG("Connecting using AUTH....")
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

bool OutgoingConnection::TokenRequest() {
	socket << id;
	socket << aes.EncryptString(token);
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
			int errCode = socket.Read<int>();
			Error(errCode, "Error while trying to connect to server: " + socket.Read<std::string>());
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
			int errCode = socket.Read<int>();
			Error(errCode, "Error while trying to connect to server: " + socket.Read<std::string>());
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
					if (!rsa->Verify(authData._GetWriteBuffer_(), signature)) {
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
			{
				int errCode = socket.Read<int>();
				Error(errCode, "Error while trying to connect to server: " + socket.Read<std::string>());
			}
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
