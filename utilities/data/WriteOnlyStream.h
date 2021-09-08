#pragma once

#include "utilities/data/Stream.h"

namespace v4d::data {
	class V4DLIB WriteOnlyStream : public Stream {
	public:

		WriteOnlyStream();
		WriteOnlyStream(size_t initialBufferSize);
		virtual ~WriteOnlyStream();

	protected:

		virtual Stream& Flush() override {return *this;}

		virtual void Send() override {};
		virtual size_t Receive(byte* data, size_t n) override {return 0;};

	};
}
