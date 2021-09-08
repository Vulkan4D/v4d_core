#include "ReadOnlyStream.h"

namespace v4d::data {
	
	ReadOnlyStream::ReadOnlyStream() : Stream(0) {}

	ReadOnlyStream::ReadOnlyStream(size_t bufferSize) : Stream(0), dataBuffer(bufferSize) {}

	ReadOnlyStream::ReadOnlyStream(const std::vector<byte>& bytes) : Stream(0), dataBuffer(bytes.size()) {
		ImportData(bytes.data(), bytes.size());
	}

	ReadOnlyStream::ReadOnlyStream(const byte* data, size_t size) : Stream(0), dataBuffer(size) {
		ImportData(data, size);
	}
	
	ReadOnlyStream::~ReadOnlyStream() {}

}
