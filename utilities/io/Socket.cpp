#include <v4d.h>

using namespace v4d::io;

#ifdef _WINDOWS
	WSADATA Socket::wsaData{};
#endif

Socket::Socket(SOCKET_TYPE type, SOCKET_PROTOCOL protocol)
	: Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol) {
	ResetSocket();
}

Socket::Socket(SOCKET socket, const sockaddr_in& remoteAddr, SOCKET_TYPE type, SOCKET_PROTOCOL protocol)
	: Stream(SOCKET_BUFFER_SIZE, /*useReadBuffer*/type==UDP), type(type), protocol(protocol), socket(socket), connected(true), remoteAddr(remoteAddr) {}

Socket::~Socket() {
	Disconnect();
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
			#ifdef _WINDOWS

				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					::send(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_DONTWAIT);
				#else
					size_t size = _GetWriteBuffer_().size();
					char data[size];
					memcpy(data, _GetWriteBuffer_().data(), size);
					if (::send(socket, data, (int)size, MSG_DONTWAIT) == -1) {
						connected = false;
					}
				#endif

			#else
				if (::send(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT) == -1) {
					connected = false;
				}
				// ::write(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size());
			#endif
		} else 
		if (IsUDP()) {
			#ifdef _WINDOWS
			
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					::sendto(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
				#else
					size_t size = _GetWriteBuffer_().size();
					char data[size];
					memcpy(data, _GetWriteBuffer_().data(), size);
					::sendto(socket, data, (int)size, MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
				#endif

			#else
				::sendto(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_CONFIRM | MSG_DONTWAIT, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
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
				// throw std::runtime_error("Error Reading Data from TCP Socket");
			} else {
				bytesRead = (size_t)rec;
			}
		} else 
		if (IsUDP()) {
			memset((char*) &incomingAddr, 0, sizeof(incomingAddr));
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
		throw std::runtime_error("Socket Disconnected");
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
					// listening = false;
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
	if (IsValid()) {
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
		// Convert IP to socket address
		#ifdef _WINDOWS
			remoteAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
		#else
			::inet_pton(protocol, host.c_str(), &remoteAddr.sin_addr);
		#endif
	}

	if (IsTCP()) {
		connected = (::connect(socket, (struct sockaddr*)&remoteAddr, sizeof remoteAddr) >= 0);
	} else {
		connected = true;
	}

	return connected;
}

bool Socket::Bind(uint16_t port, uint32_t addr) {
	if (!IsValid() || IsBound()) return false;

	if (port != 0) this->port = port;

	if (this->port != 0) {
		remoteAddr.sin_port = htons(this->port);
		remoteAddr.sin_addr.s_addr = htonl(addr);
	}
	
	// Socket Options
	::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof so_reuse);
	if (IsTCP()) {
		::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_nodelay, sizeof so_nodelay);
	}
	
	// Bind socket
	bound = (::bind(socket, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr) >= 0);
	return bound;
}

void Socket::Unbind() {
	remoteAddr.sin_port = htons(0);
	::bind(socket, (const struct sockaddr*) &remoteAddr, sizeof remoteAddr);
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
	remoteAddr.sin_family = protocol;
}

