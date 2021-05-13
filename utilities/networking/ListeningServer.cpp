#include "ListeningServer.h"
#include "utilities/io/Logger.h"
#include "utilities/crypto/SHA.h"
#include "utilities/crypto/Random.h"

using namespace v4d::networking;

ListeningServer::ListeningServer(v4d::io::SOCKET_TYPE type, std::shared_ptr<v4d::crypto::RSA> serverPrivateKey)
: listeningSocket(nullptr), rsa(serverPrivateKey) {
	listeningSocket = std::make_shared<v4d::io::Socket>(type);
	clients = std::make_shared<std::unordered_map<ulong, std::shared_ptr<IncomingClient>>>();
}

ListeningServer::ListeningServer(v4d::io::SOCKET_TYPE type, ListeningServer* src)
: listeningSocket(nullptr), rsa(src->rsa) {
	listeningSocket = std::make_shared<v4d::io::Socket>(type);
	clients = src->clients;
}

ListeningServer::~ListeningServer() {
	Stop();
}

void ListeningServer::Start(uint16_t port) {
	if (!listeningSocket->Bind(port)) {
		LOG_ERROR("ListeningServer: Failed to bind socket")
		return;
	}
	listeningSocket->StartListeningThread(listenInterval, [this](v4d::io::SocketPtr socket){
		HandleNewConnection(std::move(socket));
	});
}

void ListeningServer::Stop()  {
	listeningSocket->Disconnect();
}

bool ListeningServer::IsListening() const {
	return listeningSocket->IsListening();
}

void ListeningServer::HandleNewConnection(v4d::io::SocketPtr socket){
	try {
		// If receive nothing after timeout, Disconnect now!
		if (socket->IsTCP() && socket->Poll(newConnectionFirstByteTimeout) <= 0) {
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
		auto[clientAppName, clientAppVersion, clientType] = zapdata::ClientHello::ConstructFromStream(socket);
		if (!ValidateAppName(socket, clientAppName)) return;
		if (!ValidateVersion(socket, clientAppVersion)) return;

		// If no more data was sent, Disconnect now!
		if (socket->IsTCP() && socket->Poll(newConnectionFirstByteTimeout) <= 0) {
			LOG_ERROR_VERBOSE("ListeningServer: new connection type " << (int)clientType << " failed to send authentication request in time")
			socket->Disconnect();
			return;
		}
		
		// Client Requests
		byte request = socket->Read<byte>();
		if (socket->IsTCP()) LOG_VERBOSE("New client connection type " << (int)clientType << " request " << (int)request << " from " << socket->GetIncomingIP() << ":" << socket->GetIncomingPort())
		
		switch (request) {

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
				if (socket->IsTCP()) {
					*socket << ZAP::OK;
					*socket << (rsa? rsa->GetPublicKeyPEM() : std::string(""));
					socket->Flush();
					socket->Disconnect();
				} else {
					LOG_ERROR("ListeningServer: Received new connection request for PUBKEY over UDP")
				}
			break;
			
			case ZAP::PING:
				if (socket->IsTCP()) {
					*socket << ZAP::PONG;
					socket->Flush();
					socket->Disconnect();
				} else {
					LOG_ERROR("ListeningServer: Received new connection request for PING over UDP")
				}
			break;

			case ZAP::EXT:
				ExtendedRequest(socket, clientType);
			break;

			default:
				LOG_ERROR("ListeningServer: Received unrecognized request")
				socket->Disconnect();
		}
	} catch(v4d::io::Socket::disconnected_error&) {
		socket->Disconnect();
		LOG("Server: Client disconnected")
		return;
	}
}

bool ListeningServer::ValidateAppName(v4d::io::SocketPtr socket, uint64_t clientAppName) {
	if (GetAppName() != clientAppName) {
		if (socket->IsTCP()) {
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::APPNAME_MISMATCH, ZAP_CODES::APPNAME_MISMATCH_text};
			socket->Flush();
			socket->Disconnect();
		}
		LOG_ERROR("ListeningServer new connection : received wrong AppName")
		return false;
	}
	return true;
}

bool ListeningServer::ValidateVersion(v4d::io::SocketPtr socket, uint16_t clientVersion) {
	if (GetVersion() != clientVersion) {
		if (socket->IsTCP()) {
			*socket << ZAP::DENY;
			*socket << zapdata::Error{ZAP_CODES::VERSION_MISMATCH, ZAP_CODES::VERSION_MISMATCH_text};
			socket->Flush();
			socket->Disconnect();
		}
		LOG_ERROR("ListeningServer new connection : received wrong Version")
		return false;
	}
	return true;
}

void ListeningServer::ExtendedRequest(v4d::io::SocketPtr socket, byte /*clientType*/) {
	LOG_ERROR("ListeningServer: Received new connection extended request but has not been implemented")
	socket->Disconnect();
}

void ListeningServer::TokenRequest(v4d::io::SocketPtr socket, byte clientType) {
	uint64_t clientID = socket->Read<uint64_t>();
	{std::lock_guard lock(clientsMutex);
		if (clients->find(clientID) == clients->end()) {
			if (socket->IsTCP()) {
				clientID = 0;
				*socket << ZAP::DENY;
				*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_INVALID_ID, ZAP_CODES::AUTH_FAILED_INVALID_ID_text};
				socket->Flush();
				socket->Disconnect();
			}
			return;
		}
		auto encryptedToken = socket->ReadEncryptedStream(&clients->at(clientID)->aes);
		auto[increment, token] = zapdata::ClientToken::ConstructFromStream(encryptedToken);
		// Compare token with client's token
		if (strcmp(token.c_str(), clients->at(clientID)->token.c_str()) != 0) {
			if (socket->IsTCP()) {
				token = "";
				*socket << ZAP::DENY;
				*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_INVALID_TOKEN, ZAP_CODES::AUTH_FAILED_INVALID_TOKEN_text};
				socket->Flush();
				socket->Disconnect();
			}
			return;
		}
		// Compare encrypted increment to prevent hackers from reusing previously sent token headers to send data on behalf of someone else (some kind of nonce)
		if (increment <= clients->at(clientID)->requestIncrement || increment > clients->at(clientID)->requestIncrement + REQ_INCREMENT_MAX_DIFF) {
			if (socket->IsTCP()) {
				clientID = 0;
				*socket << ZAP::DENY;
				*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_REQ_INCREMENT, ZAP_CODES::AUTH_FAILED_REQ_INCREMENT_text};
				socket->Flush();
				socket->Disconnect();
			}
			return;
		}
		clients->at(clientID)->requestIncrement = increment;
		if (socket->IsTCP()) {
			*socket << ZAP::OK;
			socket->Flush();
		}
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AnonymousRequest(v4d::io::SocketPtr socket, byte clientType) {
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
	{std::lock_guard lock(clientsMutex);
		clients->try_emplace(clientID, std::make_shared<IncomingClient>(clientID));
	}
	if (socket->IsTCP()) {
		// Send response
		*socket << ZAP::OK;
		socket->Flush();
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::AuthRequest(v4d::io::SocketPtr socket, byte clientType) {
	// Receive AUTH data
	v4d::data::ReadOnlyStream authStream = rsa? socket->ReadEncryptedStream(rsa.get()) : socket->ReadStream();
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
	// Prepare response
	std::string token = GenerateToken();
	std::string aesHexKey = authStream.Read<std::string>();
	
	{std::lock_guard lock(clientsMutex);
		if (clients->find(clientID) != clients->end()) {
			// Destroy any existing client with the same id
			clients->erase(clientID);
		}
		(*clients)[clientID] = std::make_shared<IncomingClient>(clientID, token, aesHexKey);
		if (socket->IsTCP()) {
			v4d::data::Stream tokenAndId(10+token.size() + sizeof(clientID));
			tokenAndId << token << clientID;
			// Send response
			*socket << ZAP::OK;
			if (rsa) *socket << rsa->Sign(authData);
			socket->WriteEncryptedStream(&clients->at(clientID)->aes, tokenAndId);
		}
	}
	if (socket->IsTCP()) {
		socket->Flush();
	}
	HandleNewClient(socket, clientID, clientType);
}

void ListeningServer::HandleNewClient(v4d::io::SocketPtr socket, ulong clientID, byte clientType) {
	std::shared_ptr<IncomingClient> client = nullptr;
	{std::lock_guard lock(clientsMutex);
		if (!clientID || clients->find(clientID) == clients->end()) {
			if (socket->IsTCP()) {
				*socket << ZAP::DENY;
				*socket << zapdata::Error{ZAP_CODES::AUTH_FAILED_HANDSHAKE, ZAP_CODES::AUTH_FAILED_HANDSHAKE_text};
				socket->Flush();
				socket->Disconnect();
			}
			return;
		}
		client = clients->at(clientID);
	}
	if (socket->IsTCP()) {
		std::lock_guard lock(clientsMutex);
		// Start RunClient Thread
		client->threads.emplace_back([this,socket,client,clientType]{
			RunClient(socket, client, clientType);
		});
	} else {
		RunClient(socket, client, clientType);
	}
}

std::string ListeningServer::GenerateToken() const {
	std::vector<byte> randomBytes(50);
	v4d::crypto::Random::Generate(randomBytes);
	return v4d::crypto::SHA1(randomBytes);
}
