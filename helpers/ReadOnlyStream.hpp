#pragma once

#include <v4d.h>

namespace v4d {
	class ReadOnlyStream : public Stream {
	protected:
		std::vector<byte> dataBuffer;
		index_int dataBufferCursor;

	public:

		ReadOnlyStream() : Stream(0) {}

		ReadOnlyStream(byte* data, const size_t& size) : Stream(0), dataBufferCursor(0) {
			ImportData(data, size);
		}

		void ImportData(byte* data, const size_t& size) {
			dataBuffer.resize(size);
			memcpy(dataBuffer.data(), data, size);
		}

		INLINE size_t GetDataBufferRemaining() const {
			return dataBuffer.size() - dataBufferCursor;
		}
		
		INLINE bool IsDataBufferEnd() const {
			return dataBufferCursor == dataBuffer.size();
		}

	protected:

		virtual void Send() override {}

		virtual size_t Receive(byte* data, const size_t& n) override {
			if (n > GetDataBufferRemaining()) {
				return 0;
			}
			std::memcpy(data, dataBuffer.data() + dataBufferCursor, n);
			dataBufferCursor += n;
			return n;
		}

	};
}
