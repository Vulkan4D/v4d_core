#pragma once

#include <v4d.h>
#include <atomic>
#include <vector>
#include <thread>
#include <memory>
#include <string>

#include "utilities/data/Stream.h"
#include "utilities/data/ReadOnlyStream.h"

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

#ifndef P2P_SOCKET_BUFFER_SIZE
	#define P2P_SOCKET_BUFFER_SIZE 508
#endif

#ifdef _WINDOWS
	#define MSG_CONFIRM 0
	#define MSG_DONTWAIT 0x40
	#define SO_REUSEPORT 15
#else
	#define INVALID_SOCKET -1
#endif

namespace v4d::io {

	enum P2P_SOCKET_PROTOCOL {
		IPV4 = AF_INET,
		IPV6 = AF_INET6,
	};

	class V4DLIB P2pSocket : public v4d::data::Stream {
	protected:
		P2P_SOCKET_PROTOCOL protocol;
		SOCKET socket = INVALID_SOCKET;

		// Socket Options
		#ifdef _WINDOWS
			static WSADATA wsaData;
			char so_reuse = 1;
			char so_nodelay = 1;
		#else
			int so_reuse = 1;
			int so_nodelay = 1;
		#endif

		struct sockaddr_in boundAddr {};
		struct sockaddr_in outgoingAddr {};
		struct sockaddr_in incomingAddr {};
		socklen_t incomingAddrLen = sizeof(incomingAddr);

		virtual void Send() override;

		virtual size_t Receive(byte* data, size_t maxBytesToRead) override;

		virtual void ReadBytes_OnError(const char* str) override;

		////////////////////////////////////////////////////////////////////////////

	public:
		P2pSocket(P2P_SOCKET_PROTOCOL protocol = IPV4);
		virtual ~P2pSocket();
		
		DELETE_COPY_MOVE_CONSTRUCTORS(P2pSocket)

		void ResetSocket();
		
		inline SOCKET GetFd() const {
			return socket;
		}

		virtual std::vector<byte> GetData() override;
		
		inline P2P_SOCKET_PROTOCOL GetProtocol() const {
			return protocol;
		}

		std::string GetLastError() const;

		uint32_t GetIncomingAddr() const;
		uint16_t GetIncomingPort() const;
		std::string GetIncomingIP() const;
		
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
		
		////////////////////////////////////////////////////////////////////////////
		
		virtual bool Bind(uint16_t port, const std::string& host = "0.0.0.0");
		virtual void Unbind();
		
		template<typename T>
		void SendData(uint32_t hostAddrUInt, uint16_t port, const T& data) {
			LockWrite();
				memset(&outgoingAddr, 0, sizeof(outgoingAddr));
				outgoingAddr.sin_addr.s_addr = hostAddrUInt;
				outgoingAddr.sin_port = htons(port);
				
				*this << data;
				
				Flush();
			UnlockWrite();
		}
		template<typename T>
		void SendData(std::string host, uint16_t port, const T& data) {
			// Convert hostname to IP address
			uint32_t hostAddrUInt {};
			struct hostent* hostaddr = gethostbyname(host.c_str());
			memcpy(&hostAddrUInt, hostaddr->h_addr, hostaddr->h_length);
			SendData<T>(hostAddrUInt, port, data);
		}

		// 0 = timeout, we may continue
		// -1 = error
		// anything else = We've got data
		int Poll(int timeoutMilliseconds = 0);
	};
	
	typedef std::shared_ptr<v4d::io::P2pSocket> P2pSocketPtr;

}

