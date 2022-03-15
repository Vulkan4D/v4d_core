#include "P2pSocket.h"
#include "utilities/io/Logger.h"

using namespace v4d::io;

#ifdef _WINDOWS
	WSADATA P2pSocket::wsaData{};
#endif

P2pSocket::P2pSocket(P2P_SOCKET_PROTOCOL protocol)
	: Stream(P2P_SOCKET_BUFFER_SIZE, true), protocol(protocol) {
	ResetSocket();
}

P2pSocket::~P2pSocket() {}

std::string P2pSocket::GetIncomingIP() const {
	return std::string(inet_ntoa(incomingAddr.sin_addr));
}

uint32_t P2pSocket::GetIncomingAddr() const {
	return incomingAddr.sin_addr.s_addr;
}

uint16_t P2pSocket::GetIncomingPort() const {
	return ntohs(incomingAddr.sin_port);
}

int P2pSocket::Poll(int timeoutMilliseconds) {
	pollfd fds[1] = {pollfd{socket, POLLIN, 0}};
	int polled;
	#ifdef _WINDOWS
		polled = ::WSAPoll(fds, 1, timeoutMilliseconds);
	#else
		polled = ::poll(fds, 1, timeoutMilliseconds);
	#endif
	return polled;
}

std::string P2pSocket::GetLastError() const {
	#ifdef _WINDOWS
		// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		return std::to_string(::WSAGetLastError());
	#else
		return "";
	#endif
}

void P2pSocket::Send() {
	if (_GetWriteBuffer_().size() == 0) return;
	#ifdef _WINDOWS
		#ifdef V4D_STREAM_UNSAFE_FAST_REINTERPRET_CAST
			::sendto(socket, reinterpret_cast<const char*>(_GetWriteBuffer_().data()), (int)_GetWriteBuffer_().size(), MSG_DONTWAIT, (struct sockaddr*) &outgoingAddr, sizeof(outgoingAddr));
		#else
			size_t size = _GetWriteBuffer_().size();
			char data[size];
			memcpy(data, _GetWriteBuffer_().data(), size);
			::sendto(socket, data, (int)size, MSG_DONTWAIT, (struct sockaddr*) &outgoingAddr, sizeof(outgoingAddr));
		#endif
	#else
		::sendto(socket, _GetWriteBuffer_().data(), _GetWriteBuffer_().size(), MSG_DONTWAIT, (struct sockaddr*) &outgoingAddr, sizeof(outgoingAddr));
	#endif
}

size_t P2pSocket::Receive(byte* data, size_t maxBytesToRead) {
	if (maxBytesToRead == 0) return 0;
	ssize_t bytesRead = 0;
	memset(&incomingAddr, 0, incomingAddrLen=sizeof(incomingAddr));
	#ifdef _WINDOWS
		#ifdef V4D_STREAM_UNSAFE_FAST_REINTERPRET_CAST
			int rec = ::recvfrom(socket, reinterpret_cast<char*>(data), (int)maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &incomingAddrLen);
		#else
			char bytes[maxBytesToRead];
			int rec = ::recvfrom(socket, bytes, (int)maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &incomingAddrLen);
			memcpy(data, bytes, maxBytesToRead);
		#endif
	#else
		int rec = ::recvfrom(socket, data, maxBytesToRead, 0, (struct sockaddr*) &incomingAddr, &incomingAddrLen);
	#endif
	
	if (rec <= 0) {
		bytesRead = 0;
		// throw std::runtime_error("Error Reading Data from UDP P2pSocket");
	} else {
		bytesRead = (size_t)rec;
	}
	if (bytesRead == 0) {
		memset(data, 0, maxBytesToRead);
		// throw std::runtime_error("");
	}
	return (size_t)bytesRead;
}

void P2pSocket::ReadBytes_OnError(const char* str) {
	DEBUG_ERROR(str)
}

std::vector<byte> P2pSocket::GetData() {
	LockRead();
		// Copy and return buffer
		std::vector<byte> data(_GetReadBuffer_().size());
		memcpy(data.data(), _GetReadBuffer_().data(), _GetReadBuffer_().size());
	UnlockRead();
	return data;
};

bool P2pSocket::Bind(uint16_t port, const std::string& host) {
	if (!IsValid()) return false;

	memset((char*)&boundAddr, 0, sizeof boundAddr);
	boundAddr.sin_family = protocol;
	boundAddr.sin_port = htons(port);
	
	// Convert IP to socket address
	#ifdef _WINDOWS
		boundAddr.sin_addr.s_addr = ::inet_addr(host.c_str());
	#else
		::inet_pton(protocol, host.c_str(), &boundAddr.sin_addr);
	#endif
	
	// P2pSocket Options
	::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &so_reuse, sizeof so_reuse);
	
	// Bind socket
	return (::bind(socket, (struct sockaddr*) &boundAddr, sizeof(boundAddr)) >= 0);
}

void P2pSocket::Unbind() {
	boundAddr.sin_port = htons(0);
	::bind(socket, (struct sockaddr*) &boundAddr, sizeof(boundAddr));
}

void P2pSocket::ResetSocket() {
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
	socket = ::socket(protocol, SOCK_DGRAM, 0);
	memset(&boundAddr, 0, sizeof(boundAddr));
	memset(&outgoingAddr, 0, sizeof(outgoingAddr));
	memset(&incomingAddr, 0, sizeof(incomingAddr));
	boundAddr.sin_family = protocol;
}

