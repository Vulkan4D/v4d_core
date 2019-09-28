#include <v4d.h>

using namespace v4d::networking;

void ListeningServer::Start(uint16_t port) {
	listeningSocket.Bind(port);
	listeningSocket.StartListeningThread(listenInterval, [this](v4d::io::SharedSocket socket){
		HandleNewConnection(std::move(socket));
	});
}

void ListeningServer::HandleNewConnection(v4d::io::SharedSocket socket){
	// If receive nothing after timeout OR first byte received is not HELLO, Disconnect now!
	if (socket->Poll(newConnectionFirstByteTimeout) <= 0) {
		socket->Disconnect();
		return;
	}

	// Hello + AppName + Version + ClientType
	byte hello = socket->Read<byte>();
	if (hello != ZAP::HELLO) {
		socket->Disconnect();
		return;
	}
	if (!ValidateAppName(socket, socket->Read<std::string>())) return;
	if (!ValidateVersion(socket, socket->Read<std::string>())) return;
	byte clientType = socket->Read<byte>();

	// If no more data was sent, Disconnect now!
	if (socket->Poll(0) <= 0) {
		socket->Disconnect();
		return;
	}
	
	// Client Requests
	switch (byte request = socket->Read<byte>(); request) {

		// Authentication
		case ZAP::TOKEN:
			TokenRequest(socket, clientType);
		break;
		case ZAP::ANONYM:
			AnonymousRequest(socket, clientType);
		break;
		case ZAP::AUTH:
			AuthRequest(socket, clientType);
		break;

		// Other requests
		
		case ZAP::PING:
			socket->Write(ZAP::PONG);
			socket->Flush();
		break;

		case ZAP::EXT:
			ExtendedRequest(socket, clientType);
		break;

		default:
			LOG_ERROR("Unrecognized request")
			socket->Disconnect();
	}
}

bool ListeningServer::ValidateAppName(v4d::io::SharedSocket& socket, const std::string& clientAppName) {
	if (strcmp(GetAppName().c_str(), clientAppName.c_str()) != 0) {
		socket->Write(ZAP::DENY);
		socket->Write(ZAP_CODES::APPNAME_MISMATCH);
		socket->Write(ZAP_CODES::APPNAME_MISMATCH_text);
		socket->Flush();
		socket->Disconnect();
		return false;
	}
	return true;
}

bool ListeningServer::ValidateVersion(v4d::io::SharedSocket& socket, const std::string& clientVersion) {
	if (strcmp(GetVersion().c_str(), clientVersion.c_str()) != 0) {
		socket->Write(ZAP::DENY);
		socket->Write(ZAP_CODES::VERSION_MISMATCH);
		socket->Write(ZAP_CODES::VERSION_MISMATCH_text);
		socket->Flush();
		socket->Disconnect();
		return false;
	}
	return true;
}

void ListeningServer::ExtendedRequest(v4d::io::SharedSocket& socket, byte /*clientType*/) {
	socket->Disconnect();
}

void ListeningServer::TokenRequest(v4d::io::SharedSocket& socket, byte clientType) {
	ulong clientID = socket->Read<ulong>();
	if (clients.find(clientID) == clients.end()) {
		if (socket->IsTCP()) {
			clientID = 0;
			socket->Write(ZAP::DENY);
			socket->Write(ZAP_CODES::AUTH_FAILED_INVALID_ID);
			socket->Write(ZAP_CODES::AUTH_FAILED_INVALID_ID_text);
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	std::string token = socket->ReadEncrypted<std::string>(&clients.at(clientID)->aes);
	if (strcmp(token.c_str(), clients.at(clientID)->token.c_str()) != 0) {
		if (socket->IsTCP()) {
			token = "";
			socket->Write(ZAP::DENY);
			socket->Write(ZAP_CODES::AUTH_FAILED_INVALID_TOKEN);
			socket->Write(ZAP_CODES::AUTH_FAILED_INVALID_TOKEN_text);
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AnonymousRequest(v4d::io::SharedSocket& socket, byte clientType) {
	ulong clientID = Authenticate(nullptr);
	if (!clientID) {
		if (socket->IsTCP()) {
			socket->Write(ZAP::DENY);
			socket->Write(ZAP_CODES::DENY_ANONYMOUS);
			socket->Write(ZAP_CODES::DENY_ANONYMOUS_text);
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	clients.try_emplace(clientID, std::make_shared<IncomingClient>(clientID));
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AuthRequest(v4d::io::SharedSocket& socket, byte clientType) {
	// Receive AUTH data
	v4d::data::ReadOnlyStream authStream = rsa? socket->ReadEncryptedStream(rsa) : socket->ReadStream();
	if (rsa && authStream.GetDataBufferRemaining() == 0) {
		if (socket->IsTCP()) {
			socket->Write(ZAP::DENY);
			socket->Write(ZAP_CODES::AUTH_FAILED_RSA_DECRYPTION);
			socket->Write(ZAP_CODES::AUTH_FAILED_RSA_DECRYPTION_text);
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	auto authData = authStream.GetData();
	ulong clientID = Authenticate(&authStream);
	if (!clientID) {
		if (socket->IsTCP()) {
			socket->Write(ZAP::DENY);
			socket->Write(ZAP_CODES::AUTH_FAILED);
			socket->Write(ZAP_CODES::AUTH_FAILED_text);
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	if (socket->IsTCP()) {
		// Prepare response
		std::string token = GenerateToken();
		std::string aesHexKey = authStream.Read<std::string>();
		clients[clientID] = std::make_shared<IncomingClient>(clientID, token, aesHexKey);
		v4d::data::Stream tokenAndId(token.size() + sizeof(clientID));
		tokenAndId << token << clientID;
		// Send response
		socket->Write(ZAP::OK);
		if (rsa) socket->Write(rsa->Sign(authData));
		socket->WriteEncryptedStream(&clients.at(clientID)->aes, tokenAndId);
		socket->Flush();
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::HandleNewClient(v4d::io::SharedSocket socket, ulong clientID, byte clientType) {
	if (clientID && clients.find(clientID) != clients.end()) {
		auto client = clients.at(clientID);
		if (socket->IsTCP()) {
			// Start RunClient Thread
			client->threads.emplace_back(&ListeningServer::RunClient, this, socket, client, clientType);
		} else {
			RunClient(socket, client, clientType);
		}
	} else if (socket->IsTCP()) {
		socket->Write(ZAP::DENY);
		socket->Write(ZAP_CODES::AUTH_FAILED_HANDSHAKE);
		socket->Write(ZAP_CODES::AUTH_FAILED_HANDSHAKE_text);
		socket->Flush();
		socket->Disconnect();
	}
}

std::string ListeningServer::GenerateToken() const {
	return "123456789"; //TODO
}
