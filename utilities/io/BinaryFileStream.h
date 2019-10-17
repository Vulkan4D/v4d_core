#pragma once

#include <fstream>

namespace v4d::io {
	class V4DLIB BinaryFileStream : public v4d::data::Stream {
		std::string filePath;
		std::fstream file;

		const std::ios_base::openmode OPENMODE = std::fstream::in | std::fstream::out | std::fstream::binary;

	public:

		BinaryFileStream(const std::string& filePath, size_t bufferSize = 1024);

		virtual ~BinaryFileStream() override;

		DELETE_COPY_MOVE_CONSTRUCTORS(BinaryFileStream)

		std::vector<byte> GetData() override;

		long GetSize();
		void Truncate();
		void Reopen();

	protected:

		virtual void Send() override;
		virtual size_t Receive(byte* data, size_t n) override;

	};
}
