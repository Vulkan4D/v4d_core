#pragma once
#include <v4d.h>

namespace v4d::networking {
	class V4DLIB OutgoingConnection {
	public:
		ulong id;
		std::string token;
	protected:
		v4d::io::Socket socket;
		v4d::crypto::RSA* rsa;
		v4d::crypto::AES aes;

		bool runAsync = false;
		std::thread* asyncRunThread = nullptr;

	public:
		int connectionTimeoutMilliseconds = 2000;

		OutgoingConnection(v4d::io::SOCKET_TYPE type = v4d::io::TCP, v4d::crypto::RSA* serverPublicKey = nullptr, int aesBits = 256);
		OutgoingConnection(ulong id, std::string token, v4d::io::SOCKET_TYPE type = v4d::io::TCP, v4d::crypto::RSA* serverPublicKey = nullptr, std::string aesHex = "");
		OutgoingConnection(OutgoingConnection&);

		virtual ~OutgoingConnection();

		void Disconnect();


	// Pure-Virtual methods
		virtual std::string GetAppName() const = 0;
		virtual std::string GetVersion() const = 0;

	protected: // Pure-Virtual methods
		virtual void Authenticate(v4d::data::Stream* authStream) = 0;
		virtual void Run(v4d::io::Socket& socket) = 0;

	public:
		virtual void SendHello(byte clientType = 0);
		virtual bool Connect(std::string ip, uint16_t port, byte clientType = 1);
		virtual bool ConnectRunAsync(std::string ip, uint16_t port, byte clientType = 1);
		virtual std::string GetServerPublicKey(std::string ip, uint16_t port);

		virtual bool TokenRequest();
		virtual bool AnonymousRequest();
		virtual bool AuthRequest(v4d::data::Stream&);

		virtual bool HandleConnection();

		virtual void Error(int code, std::string message) const;

	};
}
