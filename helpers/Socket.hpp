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

		// Socket Options
		#if _WINDOWS
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

	public:
		Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE), type(type), protocol(protocol) {
			socket = ::socket(protocol, type, 0);
			memset(&remoteAddr, 0, addrLen);
			remoteAddr.sin_family = protocol;
		}

		Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol = IPV4)
		 : Stream(SOCKET_BUFFER_SIZE), type(type), protocol(protocol), socket(socket), connected(true), remoteAddr(remoteAddr) {}

		~Socket() {
			Disconnect();
		}

		virtual Socket& Flush() override {
			std::lock_guard lock(writeMutex);
			if (IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						::send(socket, reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size(), MSG_DONTWAIT);
					#else
						::send(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT);
    					// ::write(socket, writeBuffer.data(), writeBuffer.size());
					#endif
				} else 
				if (IsUDP()) {
					#if _WINDOWS
						::sendto(socket, reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
					#else
						::sendto(socket, writeBuffer.data(), writeBuffer.size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
					#endif
				}
			} else {
				//TODO error Not Connected ???
				Disconnect();
			}
			writeBuffer.resize(0);
			return *this;
		}

		virtual Socket& ReadBytes(byte* data, const size_t& n) override {
			if (IsConnected()) {
				if (IsTCP()) {
					#if _WINDOWS
						::recv(socket, reinterpret_cast<char*>(data), n, MSG_WAITALL);
					#else
						::recv(socket, data, n, MSG_WAITALL);
						// ::read(socket, data, n);
					#endif
				} else 
				if (IsUDP()) {
					memset((char*) &incomingAddr, 0, sizeof incomingAddr);
					#if _WINDOWS
						::recvfrom(socket, reinterpret_cast<char*>(data), n, MSG_WAITALL, (struct sockaddr*) &incomingAddr, &addrLen);
					#else
						::recvfrom(socket, data, n, MSG_WAITALL, (struct sockaddr*) &incomingAddr, &addrLen);
					#endif
				}
			} else {
				memset(data, 0, n);
				//TODO error Not Connected ???
				Disconnect();
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

		virtual bool Bind(short port, long addr = INADDR_ANY) {
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

		virtual bool Connect(const std::string& host, const short& port) {
			if (!IsValid() || IsBound() || IsConnected()) return false;

			remoteAddr.sin_port = htons(port);

			//TODO try all these
			// Quick method (for Linux), must include <arpa/inet.h>
				// ::inet_pton(protocol, host.c_str(), &remoteAddr.sin_addr);
			// Quick method (for Windows)
				// remoteAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
			// Complex method, no additional include
				remoteHost = gethostbyname(host.c_str());
				if (remoteHost == NULL) return false;
				memcpy((char *) &remoteAddr.sin_addr.s_addr, (char *) remoteHost->h_addr, remoteHost->h_length);

			if (IsTCP()) {
				connected = (::connect(socket, (struct sockaddr*)&remoteAddr, sizeof remoteAddr) >= 0);
			} else {
				connected = true;
			}

			return connected;
		}

		virtual void Disconnect() {
			if (listening) {
				listening = false;
				if (IsTCP()) {
					::connect(::socket(protocol, type, 0), (struct sockaddr*)&remoteAddr, sizeof remoteAddr);
				}
				listeningThread.join();
			}
			#ifdef _WINDOWS
				::closesocket(socket);
			#else
				::close(socket);
			#endif
			socket = INVALID_SOCKET;
		}

		template<class Func>
		void StartListening(Func newSocketCallback, int backlog = SOMAXCONN) {
			if (!IsBound()) return;
			if (IsTCP()) {
				listening = (::listen(socket, backlog) >= 0);
				listeningThread = std::thread([this, &newSocketCallback]{
					while (listening) {
						memset(&incomingAddr, 0, addrLen);
						SOCKET clientSocket = ::accept(socket, (struct sockaddr*)&incomingAddr, &addrLen);
						if (listening) {
							newSocketCallback(Socket(clientSocket, incomingAddr, type, protocol));
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
			}
		}

		// template<class Func>
		// void StartListeningUDP(Func newSocketCallback, int backlog = SOMAXCONN) {
		// 	if (!IsBound()) return;
		// 	if (IsUDP()) {
		// 		listening = true;
		// 		listeningThread = std::thread([this, &newSocketCallback]{
		// 			while (listening) {
		// 				newSocketCallback();
		// 			}
		// 			listening = false;
		// 		});
		// 	}
		// }

	};
}
