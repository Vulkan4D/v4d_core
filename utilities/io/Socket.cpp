#include "Socket.h"
#include "utilities/io/Logger.h"

using namespace v4d::io;

#ifdef _WINDOWS
	WSADATA Socket::wsaData{};
#endif

Socket::Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol)
	: Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol) {
	ResetSocket();
	isOriginalSocket = true;
}

Socket::Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol)
	: Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol), socket(socket), connected(true), remoteAddr(remoteAddr) {
		memset(&incomingAddr, 0, sizeof(incomingAddr));
		isOriginalSocket = false;
	}

Socket::Socket(Socket* src, SOCKET_TYPE type)
	: Socket(src->GetFd(), src->GetIncomingAddr(), type, src->GetProtocol()) {}

Socket::~Socket() {
	Disconnect();
}

std::string Socket::GetRemoteIP() const {
	return std::string(inet_ntoa(remoteAddr.sin_addr));
}

uint16_t Socket::GetRemotePort() const {
	return ntohs(remoteAddr.sin_port);
}

std::string Socket::GetIncomingIP() const {
	return IsTCP()? GetRemoteIP() : std::string(inet_ntoa(incomingAddr.sin_addr));
}

uint16_t Socket::GetIncomingPort() const {
	return IsTCP()? GetRemotePort() : ntohs(incomingAddr.sin_port);
}


int Socket::Poll(int timeoutMilliseconds) {
	pollfd fds[1] = {pollfd{socket, POLLIN, 0}};
	int polled;
	#ifdef _WINDOWS
		polled = ::WSAPoll(fds, 1, timeoutMilliseconds);
	#else
		polled = ::poll(fds, 1, timeoutMilliseconds);
	#endif
	if (polled > 0 && IsTCP()) {
		// Check if socket has been closed
		char b;
		if (::recv(socket, &b, 1, MSG_PEEK) == 0) {
			// Disconnected
			connected = false;
			return -1;
		}
	}
	return polled;
}

std::string Socket::GetLastError() const {
	#ifdef _WINDOWS
		// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		return std::to_string(::WSAGetLastError());
	#else
		return "";
	#endif
}

void Socket::Send() {
	if (_GetWriteBuffer_().size() == 0) return;
	if (IsConnected()) {
		if (IsTCP()) {
			int sent;
			try {
			#ifdef _WINDOWS
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					sent = ::send(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT);
				#else
					size_t size = _GetWriteBuffer_().size();
					char data[size];
					memcpy(data, _GetWriteBuffer_().data(), size);
					sent = ::send(socket, data, (int)size, MSG_CONFIRM | MSG_DONTWAIT);
				#endif
			#else
				sent = ::send(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT);
			#endif
			} catch (...) {
				sent = -1;
			}
			if (sent == -1) {
				connected = false;
				LOG_ERROR_VERBOSE("Disconnected in socket Send")
			}
		} else 
		if (IsUDP()) {
			#ifdef _WINDOWS
			
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					::sendto(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
				#else
					size_t size = _GetWriteBuffer_().size();
					char data[size];
					memcpy(data, _GetWriteBuffer_().data(), size);
					::sendto(socket, data, (int)size, MSG_CONFIRM | MSG_DONTWAIT, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
				#endif

			#else
				::sendto(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
			#endif
		}
	} else {
		LOG_ERROR_VERBOSE("Cannot Send Data over Socket: Not Connected")
	}
}

size_t Socket::Receive(byte* data, size_t maxBytesToRead) {
	if (maxBytesToRead == 0) return 0;
	ssize_t bytesRead = 0;
	if (IsConnected()) {
		if (IsTCP()) {
			#ifdef _WINDOWS
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					int rec = ::recv(socket, reinterpret_cast<char*>(data), (int)maxBytesToRead, MSG_WAITALL);
				#else
					char bytes[maxBytesToRead];
					int rec = ::recv(socket, bytes, (int)maxBytesToRead, MSG_WAITALL);
					memcpy(data, bytes, maxBytesToRead);
				#endif
			#else
				int rec = ::recv(socket, data, maxBytesToRead, MSG_WAITALL);
				// int rec = ::read(socket, data, maxBytesToRead);
			#endif
			
			if (rec <= 0) {
				bytesRead = 0;
				connected = false;
				LOG_ERROR_VERBOSE("Disconnected in socket Receive")
				// throw std::runtime_error("Error Reading Data from TCP Socket");
			} else {
				bytesRead = (size_t)rec;
			}
		} else 
		if (IsUDP()) {
			memset(&incomingAddr, 0, addrLen=sizeof(incomingAddr));
			#ifdef _WINDOWS
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					int rec = ::recvfrom(socket, reinterpret_cast<char*>(data), (int)maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &addrLen);
				#else
					char bytes[maxBytesToRead];
					int rec = ::recvfrom(socket, bytes, (int)maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &addrLen);
					memcpy(data, bytes, maxBytesToRead);
				#endif
			#else
				int rec = ::recvfrom(socket, data, maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &addrLen);
			#endif
			
			if (rec <= 0) {
				bytesRead = 0;
				// throw std::runtime_error("Error Reading Data from UDP Socket");
			} else {
				bytesRead = (size_t)rec;
			}
		}
	}
	if (bytesRead == 0) {
		memset(data, 0, maxBytesToRead);
		throw disconnected_error();
	}
	return (size_t)bytesRead;
}

void Socket::ReadBytes_OnError(const char* str) {
	if (logErrors) LOG_ERROR(str)
}


void Socket::StartListeningThread(int waitIntervalMilliseconds, ListeningThreadCallbackFunc&& newSocketCallback) {
	if (!IsBound()) {
		LOG_ERROR("Cannot start listening on socket: Not bound on port")
		return;
	}
	if (IsTCP()) {
		listening = (::listen(socket, SOMAXCONN) >= 0);
		if (!listening) {
			LOG_ERROR("Failed to listen on Socket")
			return;
		}
		listeningThread = new std::thread([this, waitIntervalMilliseconds](ListeningThreadCallbackFunc&& newSocketCallback){
			while (IsListening()) {

				// Check if there is a connection waiting in the socket
				int polled = Poll(waitIntervalMilliseconds);
				if (polled == 0) continue; // timeout, keep going
				if (polled == -1) { // error, stop here
					LOG_ERROR("TCP Socket Listening Poll error")
					// listening = false;
					break;
				}

				if (polled == 1) {
					// We have an incoming connection awaiting... Accept it !
					memset(&incomingAddr, 0, sizeof(incomingAddr));
					SOCKET clientSocket = ::accept(socket, (struct sockaddr*)&incomingAddr, &addrLen);
					if (IsListening() && IsValid(clientSocket)) {
						newSocketCallback(clientSockets.emplace_back(std::make_shared<Socket>(clientSocket, incomingAddr, type, protocol)));
					}
				} else {
					INVALIDCODE("polled > 1")
				}
			}
		}, std::forward<ListeningThreadCallbackFunc>(newSocketCallback));
	} else if (IsUDP()) {
		listening = true;
		listeningThread = new std::thread([this, waitIntervalMilliseconds](ListeningThreadCallbackFunc&& newSocketCallback){
			std::shared_ptr<Socket> s = std::make_shared<Socket>(socket, remoteAddr, type, protocol);
			while (IsListening()) {

				// Check if there is a connection waiting in the socket
				int polled = s->Poll(waitIntervalMilliseconds);
				if (polled == 0) continue; // timeout, keep going
				if (polled == -1) { // error, stop here
					LOG_ERROR("UDP Socket Listening Poll error")
					listening = false;
					ResetReadBuffer();
					break;
				}

				if (IsListening()) {
					newSocketCallback(s);
				}
			}
		}, std::forward<ListeningThreadCallbackFunc>(newSocketCallback));
	}
}

std::vector<byte> Socket::GetData() {
	LockRead();
		// Copy and return buffer
		std::vector<byte> data(_GetReadBuffer_().size());
		memcpy(data.data(), _GetReadBuffer_().data(), _GetReadBuffer_().size());
	UnlockRead();
	return data;
};

void Socket::Disconnect() {
	connected = false;
	StopListening();
	if (IsValid() && isOriginalSocket) {
		#ifdef _WINDOWS
			::closesocket(socket);
		#else
			::close(socket);
		#endif
		socket = INVALID_SOCKET;
	}
	for (auto& s : clientSockets) {
		s->Disconnect();
	}
	clientSockets.clear();
}

void Socket::StopListening() {
	if (bound) {
		Unbind();
	}
	listening = false;
	if (listeningThread) {
		if (listeningThread->joinable()) {
			listeningThread->join();
			delete listeningThread;
			listeningThread = nullptr;
		}
	}
}

bool Socket::Connect(const std::string& host, uint16_t port) {
	if (!IsValid()) ResetSocket();
	if (IsBound()) {
		LOG_ERROR("Socket already bound")
		return false;
	}
	if (IsTCP() && IsConnected()) {
		LOG_ERROR("Socket already connected")
		return false;
	}
	
	if (port != 0) {
		remoteAddr.sin_port = htons(port);
	}

	if (host != "") {
		
		// HostName or IP Address
			// Convert hostname to IP address
			struct hostent* hostaddr = gethostbyname(host.c_str());
			if (!hostaddr)
				return false;
			memcpy(&remoteAddr.sin_addr.s_addr, hostaddr->h_addr, hostaddr->h_length);
		
		// IP ADDRESS ONLY
			// // Convert IP to socket address
			// #ifdef _WINDOWS
			// 	remoteAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
			// #else
			// 	::inet_pton(protocol, host.c_str(), &remoteAddr.sin_addr);
			// #endif
	}

	if (IsTCP()) {
		connected = (::connect(socket, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr)) >= 0);
	} else if (IsUDP()) connected = true;
	
	return connected;
}

bool Socket::Bind(uint16_t port, const std::string& host) {
	if (!IsValid() || IsBound()) return false;

	memset((char*)&remoteAddr, 0, sizeof remoteAddr);
	remoteAddr.sin_family = protocol;
	remoteAddr.sin_port = htons(port);
	
	// Convert IP to socket address
	#ifdef _WINDOWS
		remoteAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
	#else
		::inet_pton(protocol, host.c_str(), &remoteAddr.sin_addr);
	#endif
	
	// Socket Options
	::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof so_reuse);
	if (IsTCP()) {
		::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_nodelay, sizeof so_nodelay);
	}
	
	// Bind socket
	bound = (::bind(socket, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr)) >= 0);
	return bound;
}

void Socket::Unbind() {
	remoteAddr.sin_port = htons(0);
	::bind(socket, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
	bound = false;
}

void Socket::ResetSocket() {
	#ifdef _WINDOWS
		static bool WSAinit = false;
		if (!WSAinit) {
			WSAinit = true;
			LOG_VERBOSE("WSA Startup...")
			if (::WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
				socket = INVALID_SOCKET;
				LOG_ERROR("WSA Startup Error")
				return;
			}
			LOG_VERBOSE("WSA status: " << wsaData.szSystemStatus)
			//TODO call ::WSACleanup(); at the end of the program... Or is it really necessary ? ... Maybe create a global shared_ptr singleton with destructor...
		}
	#endif
	socket = ::socket(protocol, type, 0);
	memset(&remoteAddr, 0, sizeof(remoteAddr));
	memset(&incomingAddr, 0, sizeof(incomingAddr));
	remoteAddr.sin_family = protocol;
}

