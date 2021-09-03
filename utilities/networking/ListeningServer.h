#pragma once

#include <v4d.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "utilities/crypto/RSA.h"
#include "utilities/data/ReadOnlyStream.hpp"
#include "utilities/io/Socket.h"
#include "utilities/networking/IncomingClient.h"
#include "utilities/networking/ZAP.hh"
#include "ClientPool.h"

namespace v4d::networking {

	class V4DLIB ListeningServer {
	protected:
		std::shared_ptr<ClientPool> clientPool;
		v4d::io::SocketPtr listeningSocket;
		std::shared_ptr<v4d::crypto::RSA> rsa;

		const ulong REQ_INCREMENT_MAX_DIFF = 100000; // maximum acceptable difference in the increment index between two requests
		
	public:

		ListeningServer(std::shared_ptr<ClientPool>, v4d::io::SOCKET_TYPE type = v4d::io::TCP, std::shared_ptr<v4d::crypto::RSA> serverPrivateKey = nullptr);
		ListeningServer(ListeningServer* src, v4d::io::SOCKET_TYPE type);

		virtual ~ListeningServer();

		DELETE_COPY_MOVE_CONSTRUCTORS(ListeningServer)

		int listenInterval = 10;
		int newConnectionFirstByteTimeout = 500;

		virtual void Start(uint16_t port = 0);
		virtual void Stop();
		
		virtual bool IsListening() const;

	// Pure-Virtual methods
		virtual uint64_t GetAppName() const = 0;
		virtual uint16_t GetVersion() const = 0;

	protected: // Pure-Virtual methods
		virtual IncomingClientPtr Authenticate(v4d::data::ReadOnlyStream* encryptedStream, v4d::data::ReadOnlyStream* plainStream) = 0;
		virtual void Communicate(v4d::io::SocketPtr, std::shared_ptr<IncomingClient>, byte clientType) = 0;

	protected:
		virtual void HandleNewConnection(v4d::io::SocketPtr socket);
		
		virtual bool ValidateAppName(v4d::io::SocketPtr socket, uint64_t clientAppName);
		virtual bool ValidateVersion(v4d::io::SocketPtr socket, uint16_t clientVersion);
		
		virtual void ExtendedRequest(v4d::io::SocketPtr socket, byte clientType);
		
		virtual void TokenRequest(v4d::io::SocketPtr socket, byte clientType);
		virtual void AnonymousRequest(v4d::io::SocketPtr socket, byte clientType);
		virtual void AuthRequest(v4d::io::SocketPtr socket, byte clientType);
		
		virtual void HandleNewClient(v4d::io::SocketPtr socket, IncomingClientPtr client, byte clientType);

		virtual std::string GenerateToken() const;
	};
}
