#pragma once

#include <v4d.h>

namespace v4d {
	class DataStream : public Stream {
	protected:
		index_int readBufferCursor;
		std::mutex readMutex;
		std::vector<byte> readBuffer;
		std::condition_variable readBufferCondition;

	public:

		DataStream(size_t writeBufferSize) : Stream(writeBufferSize), readBufferCursor(0) {
			readBuffer.reserve(writeBufferSize);
		}

		virtual DataStream& ReadBytes(byte* data, const size_t& n) override {
			std::unique_lock lock(readMutex);
			readBufferCondition.wait(lock, [this, n]{
				return n <= (this->readBuffer.size() - this->readBufferCursor);
			});
			std::memcpy(data, readBuffer.data() + readBufferCursor, n);
			readBufferCursor += n;
			return *this;
		}

		virtual DataStream& Flush() override {
			std::scoped_lock lock(readMutex, writeMutex);
			if (readBufferCursor == readBuffer.size()) {
				readBuffer = writeBuffer;
				readBufferCursor = 0;
			} else {
				readBuffer.insert(readBuffer.end(), writeBuffer.begin(), writeBuffer.end());
			}
			readBufferCondition.notify_one();
			writeBuffer.resize(0);
			return *this;
		}

	};
}
