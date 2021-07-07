#pragma once

#include <v4d.h>
#include <atomic>
#include <vector>
#include <thread>
#include <memory>
#include <string>

#include "utilities/data/Stream.h"
#include "utilities/data/ReadOnlyStream.hpp"

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <direct.h>
	#include <ws2tcpip.h>
	#include <mstcpip.h>
#else// _LINUX
	#include <sys/socket.h>
	#include <sys/poll.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
#endif

#ifndef SOCKET_BUFFER_SIZE
	#define SOCKET_BUFFER_SIZE 1024
#endif

#ifdef _WINDOWS
	#define MSG_CONFIRM 0
	#define MSG_DONTWAIT 0x40
	#define SO_REUSEPORT 15
#else
	#define INVALID_SOCKET -1
#endif

// https://docs.microsoft.com/en-us/windows/win32/winsock/porting-socket-applications-to-winsock

namespace v4d::io {

	enum SOCKET_TYPE : byte {
		INVALID = 0,
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM,
	};

	enum SOCKET_PROTOCOL {
		IPV4 = AF_INET,
		IPV6 = AF_INET6,
	};

	class V4DLIB Socket : public v4d::data::Stream {
	protected:
		SOCKET_TYPE type;
		SOCKET_PROTOCOL protocol;
		std::atomic<SOCKET> socket = INVALID_SOCKET;
		std::atomic<bool>	bound = false, 
							connected = false, 
							listening = false,
							logErrors = true;

		// Socket Options
		#ifdef _WINDOWS
			static WSADATA wsaData;
			char so_reuse = 1;
			char so_nodelay = 1;
		#else
			int so_reuse = 1;
			int so_nodelay = 1;
		#endif

		struct sockaddr_in remoteAddr {}; // Used for bind, connect and sending data
		struct sockaddr_in incomingAddr {}; // Used as temporary addr for receiving(UDP) and listening(TCP)
		socklen_t addrLen = sizeof(incomingAddr);
		bool isOriginalSocket = true;

		std::thread* listeningThread = nullptr;
		std::vector<std::shared_ptr<v4d::io::Socket>> clientSockets {};

		virtual void Send() override;

		virtual size_t Receive(byte* data, size_t maxBytesToRead) override;

		virtual void ReadBytes_OnError(const char* str) override;

		////////////////////////////////////////////////////////////////////////////

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4);
		Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4);
		Socket(Socket* src, SOCKET_TYPE type);
		virtual ~Socket();
		
		struct disconnected_error : public std::runtime_error {
			disconnected_error() : std::runtime_error("Socket Disconnected") {}
		};

		DELETE_COPY_MOVE_CONSTRUCTORS(Socket)

		void ResetSocket();
		
		std::vector<std::shared_ptr<v4d::io::Socket>>& GetClientSockets() {
			return clientSockets;
		}
		
		inline SOCKET GetFd() const {
			return socket;
		}

		virtual std::vector<byte> GetData() override;
		
		inline sockaddr_in GetRemoteAddr() const {
			return remoteAddr;
		}
		
		inline void SetRemoteAddr(sockaddr_in addr) {
			remoteAddr = addr;
		}
		
		inline SOCKET_TYPE GetSocketType() const {
			return type;
		}
		
		inline void SetSocketType(SOCKET_TYPE t) {
			type = t;
		}
		
		inline SOCKET_PROTOCOL GetProtocol() const {
			return protocol;
		}

		std::string GetLastError() const;

		inline void SetLogErrors(bool logErrors) {
			this->logErrors = logErrors;
		}

		inline sockaddr_in GetIncomingAddr() const {
			return incomingAddr;
		}

		std::string GetRemoteIP() const;
		uint16_t GetRemotePort() const;
		std::string GetIncomingIP() const;
		uint16_t GetIncomingPort() const;
		
		inline bool IsValid() const {
			#ifdef _WINDOWS
				return socket != INVALID_SOCKET;
			#else
				return socket >= 0 && socket != INVALID_SOCKET;
			#endif
		}
		inline static bool IsValid(SOCKET s) {
			#ifdef _WINDOWS
				return s != INVALID_SOCKET;
			#else
				return s >= 0 && s != INVALID_SOCKET;
			#endif
		}
		inline bool IsBound() const {
			return IsValid() && bound;
		}
		inline bool IsListening() const {
			return IsValid() && listening;
		}
		inline bool IsConnected() const {
			return IsValid() && connected;
		}
		inline void SetConnected(bool isConnected = true) {
			connected = isConnected;
		}
		inline bool IsTCP() const {
			return type == TCP;
		}
		inline bool IsUDP() const {
			return type == UDP;
		}

		////////////////////////////////////////////////////////////////////////////

		virtual bool Bind(uint16_t port, const std::string& host = "0.0.0.0");
		virtual void Unbind();

		virtual bool Connect(const std::string& host = "", uint16_t port = 0);

		void Disconnect();

		typedef std::function<void(std::shared_ptr<v4d::io::Socket>)> ListeningThreadCallbackFunc;
		void StartListeningThread(int waitIntervalMilliseconds, ListeningThreadCallbackFunc&&);

		void StopListening();

		// 0 = timeout, we may continue
		// -1 = error
		// anything else = We've got data
		int Poll(int timeoutMilliseconds = 0);

		inline void SetConnected() {
			connected = true;
		}
		
	};
	
	typedef std::shared_ptr<v4d::io::Socket> SocketPtr;

}

