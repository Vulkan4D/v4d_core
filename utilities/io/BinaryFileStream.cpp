#include "BinaryFileStream.h"
#include "utilities/io/Logger.h"

using namespace v4d::io;

BinaryFileStream::BinaryFileStream(const std::string& filePath, size_t bufferSize) : Stream(bufferSize), FilePath(filePath) {
	AutoCreateFile();
	Open();
 }

BinaryFileStream::~BinaryFileStream() {
	Close();
}

std::vector<byte> BinaryFileStream::GetData() {
	if (!Open()) return {};
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
	if (!Open()) return 0;
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
		Close();
		file.open(filePath, OPENMODE | std::fstream::trunc);
		if (!file.is_open()) {
			LOG_ERROR("Cannot open/truncate file '" << filePath << "'")
		}
	UnlockReadWrite();
}

bool BinaryFileStream::Open() {
	if (file.is_open()) return true;
	file.open(this->filePath, OPENMODE);
	if (file.is_open()) return true;
	LOG_ERROR("Cannot open file '" << filePath << "'")
	return false;
}

void BinaryFileStream::Close() {
	if (file.is_open()) {
		file.close();
	}
}

void BinaryFileStream::Reopen(bool seekEnd) {
	LockReadWrite();
		Close();
		file.open(filePath, seekEnd? (OPENMODE | std::fstream::ate) : OPENMODE);
		if (!file.is_open()) {
			LOG_ERROR("Cannot open/reopen file '" << filePath << "'")
		}
	UnlockReadWrite();
}

bool BinaryFileStream::Delete() {
	Close();
	return FilePath::Delete();
}

void BinaryFileStream::Send() {
	if (!Open()) return;
	LockReadWrite();
		file.write((char*)_GetWriteBuffer_().data(), (long)_GetWriteBuffer_().size());
		file.close();
	UnlockReadWrite();
}

size_t BinaryFileStream::Receive(byte* data, size_t n) {
	if (!Open()) return 0;
	LockReadWrite();
		if (!file.good()) {
			LOG_ERROR("File '" << filePath << "' not good")
		}
		file.read((char*)data, (long)n);
	UnlockReadWrite();
	return n;
}
