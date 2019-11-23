#pragma once
#include <v4d.h>

#ifndef SOCKET_BUFFER_SIZE
	#define SOCKET_BUFFER_SIZE 1024
#endif

#ifdef _WINDOWS
	#define MSG_DONTWAIT 0
	#define SO_REUSEPORT 0
#else
	#define INVALID_SOCKET -1
#endif

// https://docs.microsoft.com/en-us/windows/win32/winsock/porting-socket-applications-to-winsock

namespace v4d::io {

	enum SOCKET_TYPE {
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
		SOCKET socket = INVALID_SOCKET;
		std::atomic<bool>	bound = false, 
							connected = false, 
							listening = false,
							logErrors = true;
		uint16_t port;

		// Socket Options
		#ifdef _WINDOWS
			static WSADATA wsaData;
			char so_reuse = 1;
			char so_nodelay = 1;
		#else
			int so_reuse = 1;
			int so_nodelay = 1;
		#endif

		struct hostent *remoteHost;
		struct sockaddr_in remoteAddr; // Used for bind, connect and sending data
		struct sockaddr_in incomingAddr; // Used as temporary addr for receiving(UDP) and listening(TCP)
		socklen_t addrLen = sizeof incomingAddr;

		std::thread* listeningThread = nullptr;

		virtual void Send() override;

		virtual size_t Receive(byte* data, size_t maxBytesToRead) override;

		virtual void ReadBytes_OnError(const char* str) override;

		////////////////////////////////////////////////////////////////////////////

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4);
		Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4);
		virtual ~Socket();

		DELETE_COPY_MOVE_CONSTRUCTORS(Socket)

		void ResetSocket();

		virtual std::vector<byte> GetData() override;

		std::string GetLastError() const {
			#ifdef _WINDOWS
				// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
				return std::to_string(::WSAGetLastError());
			#else
				return "";
			#endif
		}

		void SetLogErrors(bool logErrors) {
			this->logErrors = logErrors;
		}

		sockaddr_in GetRemoteAddress() const {
			return remoteAddr;
		}

		sockaddr_in GetIncomingAddress() const {
			return incomingAddr;
		}

		std::string GetRemoteIP() const {
			return inet_ntoa(remoteAddr.sin_addr);
		}

		std::string GetIncomingIP() const {
			return inet_ntoa(incomingAddr.sin_addr);
		}

		bool IsValid() const {
			#ifdef _WINDOWS
				return socket != INVALID_SOCKET;
			#else
				return socket >= 0 && socket != INVALID_SOCKET;
			#endif
		}
		static bool IsValid(SOCKET s) {
			#ifdef _WINDOWS
				return s != INVALID_SOCKET;
			#else
				return s >= 0 && s != INVALID_SOCKET;
			#endif
		}
		bool IsBound() const {
			return IsValid() && bound;
		}
		bool IsListening() const {
			return IsValid() && listening;
		}
		bool IsConnected() const {
			return IsValid() && connected;
		}
		bool IsTCP() const {
			return type == TCP;
		}
		bool IsUDP() const {
			return type == UDP;
		}

		////////////////////////////////////////////////////////////////////////////

		virtual bool Bind(uint16_t port, uint32_t addr = INADDR_ANY);

		virtual bool Connect(const std::string& host, uint16_t port);

		void Disconnect();

		typedef std::function<void(std::shared_ptr<v4d::io::Socket>)> ListeningThreadCallbackFunc;
		void StartListeningThread(int waitIntervalMilliseconds, ListeningThreadCallbackFunc&&);

		void StopListening();

		int Poll(int timeoutMilliseconds = 0) {
			pollfd fds[1] = {pollfd{socket, POLLIN, 0}};
			#ifdef _WINDOWS
				return ::WSAPoll(fds, 1, timeoutMilliseconds);
			#else
				return ::poll(fds, 1, timeoutMilliseconds);
			#endif
		}

		void SetConnected() {
			connected = true;
		}
		
	};
	
	typedef std::shared_ptr<v4d::io::Socket> SharedSocket;

}

