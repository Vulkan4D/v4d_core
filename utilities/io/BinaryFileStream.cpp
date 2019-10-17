#include <v4d.h>

using namespace v4d::io;

BinaryFileStream::BinaryFileStream(const std::string& filePath, size_t bufferSize) : Stream(bufferSize), filePath(filePath) {
	file.open(filePath, OPENMODE);
	if (!file.is_open()) {
		file.open(filePath, OPENMODE | std::fstream::trunc);
		if (!file.is_open()) {
			LOG_ERROR("Cannot open file '" << filePath << "'")
		}
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
