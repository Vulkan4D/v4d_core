#pragma once
#include <v4d.h>

namespace v4d::io {
	class V4DLIB BinaryFileStream : public v4d::data::Stream, public v4d::io::FilePath {
		std::fstream file;

		const std::ios_base::openmode OPENMODE = std::fstream::in | std::fstream::out | std::fstream::binary;

	public:

		BinaryFileStream(const std::string& filePath, size_t bufferSize = 1024);

		virtual ~BinaryFileStream() override;

		DELETE_COPY_MOVE_CONSTRUCTORS(BinaryFileStream)

		long GetSize();
		void Truncate();
		void Reopen();

		// Stream Overrides
		std::vector<byte> GetData() override;

		// FilePath Overrides
		virtual bool Delete() override;
		
		inline bool IsEOF() const;
		
		inline long GetReadPos();
		inline long GetWritePos();

		inline void SetReadPos(long);
		inline void SetWritePos(long);

	protected:

		// Stream Overrides
		virtual void Send() override;
		virtual size_t Receive(byte* data, size_t n) override;

	};
}
