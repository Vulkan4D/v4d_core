#pragma once

#include <v4d.h>

#ifndef SOCKET_BUFFER_SIZE
	#define SOCKET_BUFFER_SIZE 1024
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
		fd socket = -1;
		bool bound = false, connected = false;
		short port;
		int so_reuse = 1;
		int so_nodelay = 1;
		struct hostent *remoteHost;
    	struct sockaddr_in serverAddr, clientAddr;

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4) : Stream(SOCKET_BUFFER_SIZE), type(type), protocol(protocol) {
			socket = ::socket(protocol, type, 0);
			memset((char*) &serverAddr, 0, sizeof(serverAddr));
			memset((char*) &clientAddr, 0, sizeof(clientAddr));
			serverAddr.sin_family = protocol;
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
					::send(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT);
				} else 
				if (IsUDP()) {
					::sendto(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
				}
			}
			writeBuffer.resize(0);
			return *this;
		}

		virtual Socket& ReadBytes(byte* data, const size_t& n) override {
			if (IsValid() && IsConnected()) {
				if (IsTCP()) {
					::recv(socket, data, n, MSG_WAITALL);
				} else 
				if (IsUDP()) {
					socklen_t addrLen;
					::recvfrom(socket, data, n, MSG_WAITALL, (struct sockaddr*) &clientAddr, &addrLen);
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

		virtual bool Bind(short port, in_addr_t addr = INADDR_ANY) {
			if (!IsValid() || IsBound()) return false;

			this->port = port;

			serverAddr.sin_port = htons(port);
			serverAddr.sin_addr.s_addr = htonl(addr);
			
			// Socket Options
			::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof(so_reuse));
			if (IsTCP()) {
				::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_nodelay, sizeof(so_nodelay));
			}
			
			// Bind socket
			bound = (::bind(socket, (const struct sockaddr*) &serverAddr, sizeof(serverAddr)) >= 0);
			return bound;
		}

		virtual bool Connect(const std::string& host, const short& port) {
			if (!IsValid() || IsBound() || IsConnected()) return false;

			serverAddr.sin_port = htons(port);

			// Quick method, must include <arpa/inet.h>
				// ::inet_pton(protocol, host.c_str(), &serverAddr.sin_addr);
			// Complex method, no additional include
				remoteHost = gethostbyname(host.c_str());
				if (remoteHost == NULL) return false;
				memcpy((char *) &serverAddr.sin_addr.s_addr, (char *) remoteHost->h_addr, remoteHost->h_length);

			if (IsTCP()) {
				connected = (::connect(socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) >= 0);
			} else {
				connected = true;
			}

			return connected;
		}

		virtual void Disconnect() {
			//TODO
		}

		virtual void StartListening() {
			if (!IsTCP()) return;
			if (!IsValid() || !IsBound()) return;
			//TODO
		}

	};
}
