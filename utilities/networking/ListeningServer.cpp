#include <v4d.h>

using namespace v4d::networking;

ListeningServer::ListeningServer(v4d::io::SOCKET_TYPE type, v4d::crypto::RSA* serverPrivateKey)
	: listeningSocket(type), rsa(serverPrivateKey) {}

ListeningServer::~ListeningServer() {
	Stop();
}

void ListeningServer::Start(uint16_t port) {
	listeningSocket.Bind(port);
	listeningSocket.StartListeningThread(listenInterval, [this](v4d::io::SharedSocket socket){
		HandleNewConnection(std::move(socket));
	});
}

void ListeningServer::Stop()  {
	listeningSocket.Disconnect();
}

void ListeningServer::HandleNewConnection(v4d::io::SharedSocket socket){
	// If receive nothing after timeout, Disconnect now!
	if (socket->Poll(newConnectionFirstByteTimeout) <= 0) {
		LOG_ERROR_VERBOSE("ListeningServer: new connection failed to send first data in time")
		socket->Disconnect();
		return;
	}

	// If first byte received is not HELLO, Disconnect now!
	byte hello = socket->Read<byte>();
	if (hello != ZAP::HELLO) {
		LOG_ERROR_VERBOSE("ListeningServer: new connection first data was not the HELLO byte")
		socket->Disconnect();
		return;
	}
	auto[clientAppName, clientAppVersion, clientType] = zapdata::ClientHello::ReadFrom(socket);
	if (!ValidateAppName(socket, clientAppName)) return;
	if (!ValidateVersion(socket, clientAppVersion)) return;

	// If no more data was sent, Disconnect now!
	if (socket->Poll(0) <= 0) {
		LOG_ERROR_VERBOSE("ListeningServer: new connection failed to send authentication request in time")
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

		case ZAP::PUBKEY:
			*socket << ZAP::OK;
			*socket << (rsa? rsa->GetPublicKeyPEM() : std::string(""));
			socket->Flush();
			socket->Disconnect();
		break;
		
		case ZAP::PING:
			*socket << ZAP::PONG;
			socket->Flush();
			socket->Disconnect();
		break;

		case ZAP::EXT:
			ExtendedRequest(socket, clientType);
		break;

		default:
			LOG_ERROR("ListeningServer: Received unrecognized request")
			socket->Disconnect();
	}
}

bool ListeningServer::ValidateAppName(v4d::io::SharedSocket& socket, const std::string& clientAppName) {
	if (strcmp(GetAppName().c_str(), clientAppName.c_str()) != 0) {
		*socket << ZAP::DENY;
		*socket << zapdata::Error{ZAP_CODES::APPNAME_MISMATCH, ZAP_CODES::APPNAME_MISMATCH_text};
		socket->Flush();
		socket->Disconnect();
		return false;
	}
	return true;
}

bool ListeningServer::ValidateVersion(v4d::io::SharedSocket& socket, const std::string& clientVersion) {
	if (strcmp(GetVersion().c_str(), clientVersion.c_str()) != 0) {
		*socket << ZAP::DENY;
		*socket << zapdata::Error{ZAP_CODES::VERSION_MISMATCH, ZAP_CODES::VERSION_MISMATCH_text};
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
	auto[clientID, increment, encryptedToken] = zapdata::ClientToken::ReadFrom(socket);
	if (clients.find(clientID) == clients.end()) {
		if (socket->IsTCP()) {
			clientID = 0;
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_INVALID_ID, ZAP_CODES::AUTH_FAILED_INVALID_ID_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	if (increment <= clients.at(clientID)->requestIncrement || increment > clients.at(clientID)->requestIncrement + REQ_INCREMENT_MAX_DIFF) {
		if (socket->IsTCP()) {
			clientID = 0;
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_REQ_INCREMENT, ZAP_CODES::AUTH_FAILED_REQ_INCREMENT_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	clients.at(clientID)->requestIncrement = increment;
	std::string token = encryptedToken.Decrypt(&clients.at(clientID)->aes);
	if (strcmp(token.c_str(), clients.at(clientID)->token.c_str()) != 0) {
		if (socket->IsTCP()) {
			token = "";
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_INVALID_TOKEN, ZAP_CODES::AUTH_FAILED_INVALID_TOKEN_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	if (socket->IsTCP()) {
		*socket << ZAP::OK;
		socket->Flush();
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AnonymousRequest(v4d::io::SharedSocket& socket, byte clientType) {
	ulong clientID = Authenticate(nullptr);
	if (!clientID) {
		if (socket->IsTCP()) {
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::DENY_ANONYMOUS, ZAP_CODES::DENY_ANONYMOUS_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	if (socket->IsTCP()) {
		clients.try_emplace(clientID, std::make_shared<IncomingClient>(clientID));
		// Send response
		*socket << ZAP::OK;
		socket->Flush();
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AuthRequest(v4d::io::SharedSocket& socket, byte clientType) {
	// Receive AUTH data
	v4d::data::ReadOnlyStream authStream = rsa? socket->ReadEncryptedStream(rsa) : socket->ReadStream();
	if (rsa && authStream.GetDataBufferRemaining() == 0) {
		if (socket->IsTCP()) {
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_RSA_DECRYPTION, ZAP_CODES::AUTH_FAILED_RSA_DECRYPTION_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	auto authData = authStream.GetData();
	ulong clientID = Authenticate(&authStream);
	if (!clientID) {
		if (socket->IsTCP()) {
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED, ZAP_CODES::AUTH_FAILED_text};
			socket->Flush();
			socket->Disconnect();
		}
		return;
	}
	if (socket->IsTCP()) {
		// Prepare response
		std::string token = GenerateToken();
		std::string aesHexKey = authStream.Read<std::string>();
		if (clients.find(clientID) != clients.end()) {
			// Destroy any existing client with the same id
			clients.erase(clientID);
		}
		clients[clientID] = std::make_shared<IncomingClient>(clientID, token, aesHexKey);
		v4d::data::Stream tokenAndId(10+token.size() + sizeof(clientID));
		tokenAndId << token << clientID;
		// Send response
		*socket << ZAP::OK;
		if (rsa) *socket << rsa->Sign(authData);
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
			client->threads.emplace_back([this,socket,client,clientType]{
				RunClient(socket, client, clientType);
			});
		} else {
			RunClient(socket, client, clientType);
		}
	} else if (socket->IsTCP()) {
		*socket << ZAP::DENY;
		*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_HANDSHAKE, ZAP_CODES::AUTH_FAILED_HANDSHAKE_text};
		socket->Flush();
		socket->Disconnect();
	}
}

std::string ListeningServer::GenerateToken() const {
	std::vector<byte> randomBytes(50);
	v4d::crypto::Random::Generate(randomBytes);
	return v4d::crypto::SHA1(randomBytes);
}
