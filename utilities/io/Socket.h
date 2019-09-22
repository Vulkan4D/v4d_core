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

	class V4DLIB Socket : public Stream {
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
		#if _WINDOWS
			WSADATA wsaData;
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

		std::thread listeningThread;

		virtual void Send() override {
			if (IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						::send(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_DONTWAIT);
					#else
						::send(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT);
						// ::write(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size());
					#endif
				} else 
				if (IsUDP()) {
					#if _WINDOWS
						::sendto(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
					#else
						::sendto(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
					#endif
				}
			} else {
				//TODO error Not Connected ???
				Disconnect();
			}
		}

		virtual size_t Receive(byte* data, size_t maxBytesToRead) override {
			ssize_t bytesRead = 0;
			if (IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						int rec = ::recv(socket, reinterpret_cast<char*>(data), (int)maxBytesToRead, MSG_WAITALL);
						if (rec <= 0) {
							// LOG_ERROR("TCP SOCKET RECEIVE ERROR: " << WSAGetLastError())
							bytesRead = 0;
						} else {
							bytesRead = (size_t)rec;
						}
					#else
						bytesRead = ::recv(socket, data, maxBytesToRead, MSG_WAITALL);
						// bytesRead = ::read(socket, data, maxBytesToRead);
					#endif
				} else 
				if (IsUDP()) {
					memset((char*) &incomingAddr, 0, sizeof incomingAddr);
					#if _WINDOWS
						int rec = ::recvfrom(socket, reinterpret_cast<char*>(data), (int)maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &addrLen);
						if (rec <= 0) {
							// LOG_ERROR("UDP SOCKET RECEIVE ERROR: " << WSAGetLastError())
							bytesRead = 0;
						} else {
							bytesRead = (size_t)rec;
						}
					#else
						bytesRead = ::recvfrom(socket, data, maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &addrLen);
					#endif
				}
			} else {
				try {
					memset(data, 0, maxBytesToRead);
				} catch (...) {}
				//TODO error Not Connected ???
				Disconnect();
			}
			return (size_t)bytesRead;
		}

		virtual void ReadBytes_OnError(const char* str) override {
			if (logErrors) LOG_ERROR(str)
		}

		////////////////////////////////////////////////////////////////////////////

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol) {
			#if _WINDOWS
				if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
					socket = INVALID_SOCKET;
					return;
				}
				// LOG("WINSOCK STATUS: " << wsaData.szSystemStatus)
			#endif
			socket = ::socket(protocol, type, 0);
			memset(&remoteAddr, 0, (size_t)addrLen);
			remoteAddr.sin_family = protocol;
		}

		Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol), socket(socket), connected(true), remoteAddr(remoteAddr) {}

		~Socket() {
			Disconnect();
		}

		std::string GetLastError() const {
			#if _WINDOWS
				// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
				return std::to_string(WSAGetLastError());
			#else
				return "";
			#endif
		}

		void SetLogErrors(bool logErrors) {
			this->logErrors = logErrors;
		}

		INLINE sockaddr_in GetRemoteAddress() const {
			return remoteAddr;
		}

		INLINE sockaddr_in GetIncomingAddress() const {
			return incomingAddr;
		}

		INLINE std::string GetRemoteIP() const {
			return inet_ntoa(remoteAddr.sin_addr);
		}

		INLINE std::string GetIncomingIP() const {
			return inet_ntoa(incomingAddr.sin_addr);
		}

		INLINE bool IsValid() const {
			#ifdef _WINDOWS
				return socket != INVALID_SOCKET;
			#else
				return socket >= 0;
			#endif
		}
		INLINE bool IsBound() const {
			return IsValid() && bound;
		}
		INLINE bool IsListening() const {
			return IsValid() && listening;
		}
		INLINE bool IsConnected() const {
			return IsValid() && connected;
		}
		INLINE bool IsTCP() const {
			return type == TCP;
		}
		INLINE bool IsUDP() const {
			return type == UDP;
		}

		////////////////////////////////////////////////////////////////////////////

		virtual bool Bind(uint16_t port, uint32_t addr = INADDR_ANY) {
			if (!IsValid() || IsBound()) return false;

			this->port = port;

			remoteAddr.sin_port = htons(port);
			remoteAddr.sin_addr.s_addr = htonl(addr);
			
			// Socket Options
			::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof so_reuse);
			if (IsTCP()) {
				::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_nodelay, sizeof so_nodelay);
			}
			
			// Bind socket
			bound = (::bind(socket, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr) >= 0);
			return bound;
		}

		virtual bool Connect(const std::string& host, uint16_t port) {
			if (!IsValid() || IsBound() || IsConnected()) return false;

			remoteAddr.sin_port = htons(port);

			// Convert IP to socket address
			#if _WINDOWS
			// Quick method (for Windows)
				remoteAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
			#else
			// Quick method (for Linux), must include <arpa/inet.h>
				::inet_pton(protocol, host.c_str(), &remoteAddr.sin_addr);
			#endif
			// Complex method, no additional include
				// remoteHost = gethostbyname(host.c_str());
				// if (remoteHost == NULL) return false;
				// memcpy((char *) &remoteAddr.sin_addr.s_addr, (char *) remoteHost->h_addr, remoteHost->h_length);

			if (IsTCP()) {
				connected = (::connect(socket, (struct sockaddr*)&remoteAddr, sizeof remoteAddr) >= 0);
			} else {
				connected = true;
			}

			return connected;
		}

		virtual void Disconnect() {
			StopListening();
			if (listeningThread.joinable()) {
				listeningThread.join();
			}
			#ifdef _WINDOWS
				::closesocket(socket);
				WSACleanup();
			#else
				::close(socket);
			#endif
			socket = INVALID_SOCKET;
		}

		virtual void StopListening() {
			if (listening) {
				listening = false;
				if (IsTCP()) {
					//TODO find a better way to stop listening on TCP socket
					::connect(::socket(protocol, type, 0), (struct sockaddr*)&remoteAddr, sizeof remoteAddr);
				}
			}
		}

		template<class Func, class ...FuncArgs>
		void StartListeningThread(Func newSocketCallback, FuncArgs&&... args) {
			if (!IsBound()) return;
			if (IsTCP()) {
				listening = (::listen(socket, SOMAXCONN) >= 0);
				listeningThread = std::thread([this, &newSocketCallback, &args...]{
					while (listening) {
						memset(&incomingAddr, 0, (size_t)addrLen);
						SOCKET clientSocket = ::accept(socket, (struct sockaddr*)&incomingAddr, &addrLen);
						if (listening) {
							Socket s(clientSocket, incomingAddr, type, protocol);
							newSocketCallback(s, args...);
						} else {
							#ifdef _WINDOWS
								::closesocket(clientSocket);
							#else
								::close(clientSocket);
							#endif
						}
					}
					listening = false;
				});
			} else if (IsUDP()) {
				listening = true;
				listeningThread = std::thread([this, &newSocketCallback, &args...]{
					Socket s(socket, remoteAddr, type, protocol);
					while (listening) {
						newSocketCallback(s, args...);
					}
					listening = false;
				});
			}
		}

		void SetConnected() {
			connected = true;
		}
		

		////////////////////////////////////////////////////////////////////////////////
		// Other Read/Write methods

		// Stream
		ReadOnlyStream ReadStream();
		void WriteStream(Stream& stream);


	};
}

