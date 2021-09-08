#include "WriteOnlyStream.h"

namespace v4d::data {
	WriteOnlyStream::WriteOnlyStream() : Stream(0) {}
	WriteOnlyStream::WriteOnlyStream(size_t initialBufferSize) : Stream(0) {}
	WriteOnlyStream::~WriteOnlyStream() {}
}
