// Pre-compiled common header
#include <common/pch.hh>

// V4D Core Header
#include <v4d.h>

using namespace v4d::io;

BinaryFileStream::BinaryFileStream(const std::string& filePath, size_t bufferSize) : Stream(bufferSize), FilePath(filePath) {
	AutoCreateFile();
	file.open(this->filePath, OPENMODE);
	if (!file.is_open()) {
		LOG_ERROR("Cannot open file '" << filePath << "'")
	}
}

BinaryFileStream::~BinaryFileStream() {
	if (!file.is_open()) {
		file.close();
	}
}

std::vector<byte> BinaryFileStream::GetData() {
	std::scoped_lock lock(writeMutex, readMutex);
	long pos = (long)file.tellg();
	file.seekg(0, std::ios::end);
	long size = (long)file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<byte> data((size_t)size);
	file.read((char*)data.data(), size);
	file.seekg(pos);
	return data;
};

long BinaryFileStream::GetSize() {
	std::scoped_lock lock(writeMutex, readMutex);
	long pos = (long)file.tellg();
	file.seekg(0, std::ios::end);
	long size = (long)file.tellg();
	file.seekg(pos);
	return size;
}

bool BinaryFileStream::IsEOF() const {
	return file.eof();
}

long BinaryFileStream::GetReadPos() {
	return file.tellg();
}

long BinaryFileStream::GetWritePos() {
	return file.tellp();
}

void BinaryFileStream::SetReadPos(long pos) {
	file.seekg(pos);
}

void BinaryFileStream::SetWritePos(long pos) {
	file.seekp(pos);
}

void BinaryFileStream::Truncate() {
	std::scoped_lock lock(writeMutex, readMutex);
	if (file.is_open()) {
		file.close();
	}
	file.open(filePath, OPENMODE | std::fstream::trunc);
	if (!file.is_open()) {
		LOG_ERROR("Cannot open/truncate file '" << filePath << "'")
	}
}

void BinaryFileStream::Reopen() {
	std::scoped_lock lock(writeMutex, readMutex);
	if (file.is_open()) {
		file.close();
	}
	file.open(filePath, OPENMODE);
	if (!file.is_open()) {
		LOG_ERROR("Cannot open/reopen file '" << filePath << "'")
	}
}

bool BinaryFileStream::Delete() {
	if (file.is_open()) {
		file.close();
	}
	return FilePath::Delete();
}

void BinaryFileStream::Send() {
	std::scoped_lock lock(writeMutex, readMutex);
	if (!file.is_open()) {
		LOG_ERROR("File '" << filePath << "' not opened")
	}
	file.write((char*)_GetWriteBuffer_().data(), (long)_GetWriteBuffer_().size());
	// Reopen
	file.close();
	file.open(filePath, OPENMODE);
}

size_t BinaryFileStream::Receive(byte* data, size_t n) {
	std::scoped_lock lock(writeMutex, readMutex);
	if (!file.is_open()) {
		LOG_ERROR("File '" << filePath << "' not opened")
	}
	if (!file.good()) {
		LOG_ERROR("File '" << filePath << "' not good")
	}
	file.read((char*)data, (long)n);
	return n;
}
