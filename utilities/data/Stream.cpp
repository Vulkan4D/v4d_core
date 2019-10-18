// Pre-compiled common header
#include <common/pch.hh>

// V4D Core Header
#include <v4d.h>

using namespace v4d::data;

Stream& Stream::ReadBytes(byte* data, size_t n) {
	std::lock_guard lock(readMutex);
	if (useReadBuffer) {
		// Reset read buffer if we have read it completely
		if (readBufferCursor == readBuffer.size()) {
			ResetReadBuffer();
		}
		size_t remainingDataInBuffer = readBuffer.size() - readBufferCursor;
		// If we are missing data from buffer, fetch more from source
		if (remainingDataInBuffer < n) {
			size_t remainingDataToReceive = n - remainingDataInBuffer;
			size_t maxReadSize = readBuffer.capacity() - readBufferCursor;
			// If we dont have enough space left in our buffer, remove elements that are already read
			if (maxReadSize < remainingDataToReceive) {
				if (readBufferCursor == readBuffer.size()) {
					// If we were finished with the buffer, we should reset it
					maxReadSize = readBuffer.capacity();
				} else {
					// Erase only the part that we already read from the buffer
					readBuffer.erase(readBuffer.begin(), readBuffer.begin() + (long)readBufferCursor);
					maxReadSize = readBuffer.capacity() - readBuffer.size();
				}
				readBufferCursor = 0;
				// If we still dont have enough space left in our buffer, ERROR
				if (maxReadSize < remainingDataToReceive) {
					ReadBytes_OnError("Stream ReadBuffer capacity exceeded");
					ResetReadBuffer();
					std::memset(data, 0, n);
					return *this;
				}
			}
			// temporary resize the buffer to fit new data
			readBuffer.resize(readBufferCursor + maxReadSize);
			// Actually receive the new data (and put it directly in the buffer)
			size_t bytesRead = Receive(readBuffer.data(), maxReadSize);
			if (bytesRead == 0) {
				ReadBytes_OnError("Stream ReadBuffer received 0 bytes");
				ResetReadBuffer();
				std::memset(data, 0, n);
				return *this;
			}
			// Resize the buffer to its final size to include the new data received
			if (bytesRead > maxReadSize) {
				ReadBytes_OnError("Stream Receive bytesRead exceeded maxReadSize");
				ResetReadBuffer();
				std::memset(data, 0, n);
				return *this;
			} else if (bytesRead != maxReadSize) {
				readBuffer.resize(readBufferCursor + bytesRead);
			}
		}
		std::memcpy(data, readBuffer.data() + readBufferCursor, n);
		readBufferCursor += n;
	} else {
		Receive(data, n);
	}
	return *this;
}

// Read Another Stream (Streamception)
ReadOnlyStream Stream::ReadStream() {
	std::lock_guard lock(readMutex);
	size_t size = ReadSize();
	byte data[size];
	ReadBytes(data, size);
	return ReadOnlyStream(data, size);
}

// Encrypted Stream
ReadOnlyStream Stream::ReadEncryptedStream(v4d::crypto::Crypto* crypto) {
	return crypto->DecryptStream(Read<std::vector, byte>());
}
void Stream::WriteEncryptedStream(v4d::crypto::Crypto* crypto, Stream& stream) {
	Write<std::vector, byte>(crypto->EncryptStream(stream));
}

