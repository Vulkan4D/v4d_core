#pragma once

#include <cstring>
#include "utilities/data/Stream.h"

namespace v4d::data {
	class V4DLIB ReadOnlyStream : public Stream {
	protected:
		std::vector<byte> dataBuffer{};
		size_t dataBufferCursor = 0;

	public:

		ReadOnlyStream();
		ReadOnlyStream(size_t bufferSize);
		ReadOnlyStream(const std::vector<byte>& bytes);
		ReadOnlyStream(const byte* data, size_t size);
		virtual ~ReadOnlyStream();

		void ImportData(const byte* data, size_t size) {
			dataBuffer.resize(size);
			memcpy(dataBuffer.data(), data, size);
		}

		size_t GetDataBufferRemaining() const {
			return dataBuffer.size() - dataBufferCursor;
		}
		
		bool IsDataBufferEnd() const {
			return dataBufferCursor == dataBuffer.size();
		}

		virtual std::vector<byte> GetData() override {
			LockRead();
				// Copy and return buffer
				std::vector<byte> buf = dataBuffer;
			UnlockRead();
			return buf;
		}

		std::vector<byte>& _GetReadBuffer_() override {
			return dataBuffer;
		}

	protected:

		virtual void Send() override {}

		virtual size_t Receive(byte* data, size_t n) override {
			if (n > GetDataBufferRemaining()) {
				return 0;
			}
			std::memcpy(data, dataBuffer.data() + dataBufferCursor, n);
			dataBufferCursor += n;
			return n;
		}

	};
}
