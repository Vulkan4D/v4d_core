#include <v4d.h>

using namespace v4d::io;

v4d::ReadOnlyStream Socket::ReadStream() {
	std::lock_guard lock(readMutex);
	size_t size = ReadSize();
	byte data[size];
	ReadBytes(data, size);
	return ReadOnlyStream(data, size);
}

void Socket::WriteStream(v4d::Stream& stream) {
	std::lock_guard lock(writeMutex);
	stream.LockWrite();
	size_t size(stream._GetWriteBuffer_().size());
	WriteSize(size);
	WriteBytes(stream._GetWriteBuffer_().data(), size);
	stream.UnlockWrite();
}

