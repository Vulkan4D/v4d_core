#pragma once

#include <v4d.h>
#include <string>
#include <fstream>
#include <vector>
#include "utilities/io/FilePath.h"
#include "utilities/data/Stream.h"

namespace v4d::io {
	class V4DLIB BinaryFileStream : public v4d::data::Stream, public v4d::io::FilePath {
		std::fstream file;

		const std::ios_base::openmode OPENMODE = std::fstream::in | std::fstream::out | std::fstream::binary;

	public:

		BinaryFileStream(const std::string& filePath, size_t bufferSize = 1024);

		virtual ~BinaryFileStream() override;

		DELETE_COPY_MOVE_CONSTRUCTORS(BinaryFileStream)

		size_t GetSize();
		void Truncate();
		bool Open();
		void Close();
		void Reopen(bool seekEnd = false);

		// Stream Overrides
		std::vector<byte> GetData() override;

		// FilePath Overrides
		virtual bool Delete() override;
		
		inline bool IsEOF() const {
			return file.eof();
		}

		inline long GetReadPos() {
			return file.tellg();
		}

		inline long GetWritePos() {
			return file.tellp();
		}

		inline void SetReadPos(long pos) {
			file.seekg(pos);
		}

		inline void SetWritePos(long pos) {
			file.seekp(pos);
		}
		
		inline operator bool() const {
			return file.good();
		}

	protected:

		// Stream Overrides
		virtual void Send() override;
		virtual size_t Receive(byte* data, size_t n) override;

	};
}
