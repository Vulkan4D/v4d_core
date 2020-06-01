#pragma once
#include <v4d.h>

namespace v4d::data {
	class V4DLIB ReadOnlyStream : public Stream {
	protected:
		std::vector<byte> dataBuffer{};
		size_t dataBufferCursor = 0;

	public:

		ReadOnlyStream() : Stream(0) {}

		ReadOnlyStream(std::vector<byte> bytes) : ReadOnlyStream(bytes.data(), bytes.size()) {}

		ReadOnlyStream(byte* data, size_t size) : Stream(0) {
			ImportData(data, size);
		}

		void ImportData(byte* data, size_t size) {
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
