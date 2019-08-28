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

namespace v4d {

	enum SOCKET_TYPE {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM,
	};

	enum SOCKET_PROTOCOL {
		IPV4 = AF_INET,
		IPV6 = AF_INET6,
	};

	class Socket : public Stream {
	protected:
		SOCKET_TYPE type;
		SOCKET_PROTOCOL protocol;
		SOCKET socket = INVALID_SOCKET;
		std::atomic<bool>	bound = false, 
							connected = false, 
							listening = false;
		short port;
		char so_reuse = 1;
		char so_nodelay = 1;
		struct hostent *remoteHost;
    	struct sockaddr_in remoteAttr; // Used for bind, connect and sending data
    	struct sockaddr_in incomingAddr; // Used as temporary addr for receiving(UDP) and listening(TCP)

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE), type(type), protocol(protocol) {
			socket = ::socket(protocol, type, 0);
			memset((char*) &remoteAttr, 0, sizeof(remoteAttr));
			remoteAttr.sin_family = protocol;
		}

		Socket(SOCKET socket, const sockaddr_in& addr, int addrLen, SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE), type(type), protocol(protocol), socket(socket) {
			memset((char*) &remoteAttr, 0, sizeof(remoteAttr));
			memcpy((char*) &remoteAttr, &addr, addrLen);
		}

		~Socket() {
			if (IsValid()) {
				#ifdef _WINDOWS
					::closesocket(socket);
				#else
					::close(socket);
				#endif
			}
		}

		virtual Socket& Flush() override {
			std::lock_guard lock(writeMutex);
			if (IsValid() && IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						::send(socket, reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size(), MSG_DONTWAIT);
					#else
						::send(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT);
					#endif
				} else 
				if (IsUDP()) {
					#if _WINDOWS
						::sendto(socket, reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAttr, sizeof(remoteAttr));
					#else
						::sendto(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAttr, sizeof(remoteAttr));
					#endif
				}
			}
			writeBuffer.resize(0);
			return *this;
		}

		virtual Socket& ReadBytes(byte* data, const size_t& n) override {
			if (IsValid() && IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						::recv(socket, reinterpret_cast<char*>(data), n, MSG_WAITALL);
					#else
						::recv(socket, data, n, MSG_WAITALL);
					#endif
				} else 
				if (IsUDP()) {
					memset((char*) &incomingAddr, 0, sizeof(incomingAddr));
					socklen_t addrLen;
					#if _WINDOWS
						::recvfrom(socket, reinterpret_cast<char*>(data), n, MSG_WAITALL, (struct sockaddr*) &incomingAddr, &addrLen);
					#else
						::recvfrom(socket, data, n, MSG_WAITALL, (struct sockaddr*) &incomingAddr, &addrLen);
					#endif
				}
			}
			return *this;
		}

		////////////////////////////////////////////////////////////////////////////

		INLINE bool IsValid() const {
			#ifdef _WINDOWS
				return socket != INVALID_SOCKET;
			#else
				return socket >= 0;
			#endif
		}
		INLINE bool IsBound() const {
			return bound;
		}
		INLINE bool IsConnected() const {
			return connected;
		}
		INLINE bool IsTCP() const {
			return type == TCP;
		}
		INLINE bool IsUDP() const {
			return type == UDP;
		}

		////////////////////////////////////////////////////////////////////////////

		virtual bool Bind(short port, long addr = INADDR_ANY) {
			if (!IsValid() || IsBound()) return false;

			this->port = port;

			remoteAttr.sin_port = htons(port);
			remoteAttr.sin_addr.s_addr = htonl(addr);
			
			// Socket Options
			::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof(so_reuse));
			if (IsTCP()) {
				::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_nodelay, sizeof(so_nodelay));
			}
			
			// Bind socket
			bound = (::bind(socket, (const struct sockaddr*) &remoteAttr, sizeof(remoteAttr)) >= 0);
			return bound;
		}

		virtual bool Connect(const std::string& host, const short& port) {
			if (!IsValid() || IsBound() || IsConnected()) return false;

			remoteAttr.sin_port = htons(port);

			//TODO try all these
			// Quick method (for Linux), must include <arpa/inet.h>
				// ::inet_pton(protocol, host.c_str(), &remoteAttr.sin_addr);
			// Quick method (for Windows)
				// remoteAttr.sin_addr.s_addr = ::inet_addr(host.c_str());
			// Complex method, no additional include
				remoteHost = gethostbyname(host.c_str());
				if (remoteHost == NULL) return false;
				memcpy((char *) &remoteAttr.sin_addr.s_addr, (char *) remoteHost->h_addr, remoteHost->h_length);

			if (IsTCP()) {
				connected = (::connect(socket, (struct sockaddr*)&remoteAttr, sizeof(remoteAttr)) >= 0);
			} else {
				connected = true;
			}

			return connected;
		}

		virtual void Disconnect() {
			//TODO
		}

		virtual void StartListening(std::function<void(Socket)> newSocketCallback, int backlog = SOMAXCONN) {
			if (!IsValid() || !IsBound()) return;
			if (IsTCP()) {
				listening = (::listen(socket, backlog) >= 0);
				while (listening) {
					memset((char*) &incomingAddr, 0, sizeof(incomingAddr));
					socklen_t addrLen;
					SOCKET clientSocket = ::accept(socket, (struct sockaddr*)&incomingAddr, &addrLen);
					newSocketCallback(Socket(clientSocket, incomingAddr, addrLen, type, protocol));
				}
			}
			listening = false;
		}

	};
}
