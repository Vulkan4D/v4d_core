#include "BinaryFileStream.h"
#include "utilities/io/Logger.h"

using namespace v4d::io;

BinaryFileStream::BinaryFileStream(const std::string& filePath, size_t bufferSize) : Stream(bufferSize), FilePath(filePath) {
	AutoCreateFile();
	file.open(this->filePath, OPENMODE);
	if (!file.is_open()) {
		LOG_ERROR("Cannot open file '" << filePath << "'")
	}
}

BinaryFileStream::~BinaryFileStream() {
	if (file.is_open()) {
		file.close();
	}
}

std::vector<byte> BinaryFileStream::GetData() {
	LockReadWrite();
		long pos = (long)file.tellg();
		file.seekg(0, std::ios::end);
		long size = (long)file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<byte> data((size_t)size);
		file.read((char*)data.data(), size);
		file.seekg(pos);
	UnlockReadWrite();
	return data;
};

long BinaryFileStream::GetSize() {
	LockReadWrite();
		long pos = (long)file.tellg();
		file.seekg(0, std::ios::end);
		long size = (long)file.tellg();
		file.seekg(pos);
	UnlockReadWrite();
	return size;
}

void BinaryFileStream::Truncate() {
	LockReadWrite();
		if (file.is_open()) {
			file.close();
		}
		file.open(filePath, OPENMODE | std::fstream::trunc);
		if (!file.is_open()) {
			LOG_ERROR("Cannot open/truncate file '" << filePath << "'")
		}
	UnlockReadWrite();
}

void BinaryFileStream::Reopen(bool seekEnd) {
	LockReadWrite();
		if (file.is_open()) {
			file.close();
		}
		file.open(filePath, seekEnd? (OPENMODE | std::fstream::ate) : OPENMODE);
		if (!file.is_open()) {
			LOG_ERROR("Cannot open/reopen file '" << filePath << "'")
		}
	UnlockReadWrite();
}

bool BinaryFileStream::Delete() {
	if (file.is_open()) {
		file.close();
	}
	return FilePath::Delete();
}

void BinaryFileStream::Send() {
	LockReadWrite();
		if (!file.is_open()) {
			LOG_ERROR("File '" << filePath << "' not opened")
		}
		file.write((char*)_GetWriteBuffer_().data(), (long)_GetWriteBuffer_().size());
		// Reopen
		file.close();
		file.open(filePath, OPENMODE);
	UnlockReadWrite();
}

size_t BinaryFileStream::Receive(byte* data, size_t n) {
	LockReadWrite();
		if (!file.is_open()) {
			LOG_ERROR("File '" << filePath << "' not opened")
		}
		if (!file.good()) {
			LOG_ERROR("File '" << filePath << "' not good")
		}
		file.read((char*)data, (long)n);
	UnlockReadWrite();
	return n;
}
