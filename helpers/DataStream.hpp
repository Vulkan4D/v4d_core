#pragma once

#include <v4d.h>

namespace v4d {
	class DataStream : public Stream {
	protected:
		size_t dataBufferCursor;
		std::vector<byte> dataBuffer;
		std::mutex dataWaitMutex;
		std::condition_variable dataBufferWaitCondition;

	public:

		DataStream(size_t bufferSize) : Stream(bufferSize), dataBufferCursor(0) {
			dataBuffer.reserve(bufferSize);
		}

		DataStream(byte* data, const size_t& size) : Stream(size) {
			dataBuffer.resize(size);
			memcpy(dataBuffer.data(), data, size);
		}

		DELETE_COPY_MOVE_CONSTRUCTORS(DataStream)

	protected:

		INLINE size_t GetDataBufferRemaining() const {
			return dataBuffer.size() - dataBufferCursor;
		}
		
		INLINE bool IsDataBufferEnd() const {
			return dataBufferCursor == dataBuffer.size();
		}

		virtual void Send() override {
			std::scoped_lock lock(dataWaitMutex);
			if (IsDataBufferEnd()) {
				dataBuffer = _GetWriteBuffer_();
				dataBufferCursor = 0;
			} else {
				dataBuffer.insert(dataBuffer.end(), _GetWriteBuffer_().begin(), _GetWriteBuffer_().end());
			}
			dataBufferWaitCondition.notify_one();
		}

		virtual size_t Receive(byte* data, size_t n) override {
			std::unique_lock lock(dataWaitMutex);
			dataBufferWaitCondition.wait(lock, [this, n]{
				return n <= this->GetDataBufferRemaining();
			});
			std::memcpy(data, dataBuffer.data() + dataBufferCursor, n);
			dataBufferCursor += n;
			return n;
		}

	};
}
