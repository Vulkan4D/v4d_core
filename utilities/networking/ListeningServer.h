#pragma once

#include <v4d.h>

namespace v4d::networking {

	class V4DLIB ListeningServer {
	protected:
		const ulong REQ_INCREMENT_MAX_DIFF = 100000; // maximum acceptable difference in the increment index between two requests
		v4d::io::Socket listeningSocket;
		v4d::crypto::RSA* rsa;

		std::unordered_map<ulong, std::shared_ptr<IncomingClient>> clients{};

	public:

		ListeningServer(v4d::io::SOCKET_TYPE type = v4d::io::TCP, v4d::crypto::RSA* serverPrivateKey = nullptr)
		 : listeningSocket(type), rsa(serverPrivateKey) {}

		virtual ~ListeningServer() {
			Stop();
		}

		DELETE_COPY_MOVE_CONSTRUCTORS(ListeningServer)

		int listenInterval = 10;
		int newConnectionFirstByteTimeout = 500;

		void Start(uint16_t port);

		void Stop() {
			listeningSocket.Disconnect();
		}

	// Pure-Virtual methods
		virtual std::string GetAppName() const = 0;
		virtual std::string GetVersion() const = 0;

	protected: // Pure-Virtual methods
		virtual ulong Authenticate(v4d::data::ReadOnlyStream* /*authStream*/) = 0;
		virtual void RunClient(v4d::io::SharedSocket, std::shared_ptr<IncomingClient>, byte /*clientType*/) = 0;

	protected:
		virtual void HandleNewConnection(v4d::io::SharedSocket socket);
		
		virtual bool ValidateAppName(v4d::io::SharedSocket& socket, const std::string& clientAppName);
		virtual bool ValidateVersion(v4d::io::SharedSocket& socket, const std::string& clientVersion);
		
		virtual void ExtendedRequest(v4d::io::SharedSocket& socket, byte clientType);
		
		virtual void TokenRequest(v4d::io::SharedSocket& socket, byte clientType);
		virtual void AnonymousRequest(v4d::io::SharedSocket& socket, byte clientType);
		virtual void AuthRequest(v4d::io::SharedSocket& socket, byte clientType);

		virtual void HandleNewClient(v4d::io::SharedSocket socket, ulong clientID, byte clientType);

		virtual std::string GenerateToken() const;
	};
}
