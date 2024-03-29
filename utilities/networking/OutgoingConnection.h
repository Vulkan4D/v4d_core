#pragma once

#include <v4d.h>
#include <string>
#include <thread>
#include "utilities/data/Stream.h"
#include "utilities/io/Socket.h"
#include "utilities/crypto/AES.h"
#include "utilities/crypto/RSA.h"
#include "utilities/networking/ZAP.hh"

namespace v4d::networking {
	class V4DLIB OutgoingConnection {
	public:
		int32_t id;
		std::string token;
	protected:
		v4d::io::SocketPtr socket;
		std::shared_ptr<v4d::crypto::RSA> rsa;
		v4d::crypto::AES aes;

		bool runAsync = false;
		std::thread* asyncRunThread = nullptr;

	public:
		int connectionTimeoutMilliseconds = 2000;

		OutgoingConnection(v4d::io::SOCKET_TYPE type = v4d::io::TCP, std::shared_ptr<v4d::crypto::RSA> serverPublicKey = nullptr, int aesBits = 256);
		OutgoingConnection(int32_t id, std::string token, v4d::io::SOCKET_TYPE type = v4d::io::TCP, std::shared_ptr<v4d::crypto::RSA> serverPublicKey = nullptr, std::string aesHex = "");
		OutgoingConnection(OutgoingConnection* src);
		OutgoingConnection(OutgoingConnection* src, v4d::io::SOCKET_TYPE type);

		virtual ~OutgoingConnection();

		DELETE_COPY_MOVE_CONSTRUCTORS(OutgoingConnection)

		virtual void Disconnect();

		inline void SetAsync() {
			runAsync = true;
		}
		
	protected: // Pure-Virtual methods
		virtual uint64_t GetAppName() const = 0;
		virtual uint16_t GetVersion() const = 0;
		virtual void Authenticate(v4d::data::Stream* encryptedStream, v4d::data::Stream* plainStream) = 0;
		virtual void Communicate(v4d::io::SocketPtr socket) = 0;

	public:
		virtual void SendHello(byte clientType = 0);
		virtual bool Connect(std::string ip = "", uint16_t port = 0, byte clientType = 1);
		virtual bool ConnectCommunicateAsync(std::string ip = "", uint16_t port = 0, byte clientType = 1);
		virtual std::string GetServerPublicKey(std::string ip, uint16_t port);

		virtual bool TokenRequest();
		virtual bool AnonymousRequest();
		virtual bool AuthRequest(v4d::data::Stream& encrypted, v4d::data::Stream& plain);

		virtual bool HandleConnection();

		virtual void Error(int code, std::string message) const;

	};
}
