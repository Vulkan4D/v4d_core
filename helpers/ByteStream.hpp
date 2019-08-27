#pragma once

#include <v4d.h>

namespace v4d {
	class ByteStream {
	protected:
		std::mutex readMutex, writeMutex;
		std::vector<byte> readBuffer, writeBuffer;
		std::condition_variable readBufferCondition;
		index_int readBufferCursor;

	public:

		ByteStream(size_t writeBufferSize) {
			writeBuffer.reserve(writeBufferSize);
			readBuffer.reserve(writeBufferSize);
			readBufferCursor = 0;
		}

		INLINE virtual void WriteBytes(const byte* data, const size_t& n) {
			std::lock_guard lock(writeMutex);
			if ((writeBuffer.size() + n) > writeBuffer.capacity()) {
				Flush();
			}
			writeBuffer.insert(writeBuffer.end(), data, data+n);
		}

		INLINE virtual void ReadBytes(byte* data, const size_t& n) {
			std::unique_lock lock(readMutex);
			readBufferCondition.wait(lock, [this, n]{
				return n <= (this->readBuffer.size() - this->readBufferCursor);
			});
			std::memcpy(data, readBuffer.data() + readBufferCursor, n);
			readBufferCursor += n;
		}

		INLINE virtual void Flush() {
			std::scoped_lock lock(readMutex, writeMutex);
			if (readBufferCursor == readBuffer.size()) {
				readBuffer = writeBuffer;
				readBufferCursor = 0;
			} else {
				readBuffer.insert(readBuffer.end(), writeBuffer.begin(), writeBuffer.end());
			}
			writeBuffer.resize(0);
			readBufferCondition.notify_one();
		}


		// Read/Write methods
		template<typename T>
		INLINE ByteStream& Write(const T& data) {
			WriteBytes(reinterpret_cast<const byte*>(&data), sizeof(T));
			return *this;
		}
		template<typename T>
		INLINE ByteStream& Read(T& data) {
			ReadBytes(reinterpret_cast<byte*>(&data), sizeof(T));
			return *this;
		}


		// Return-Read
		template<typename T>
		INLINE T Read() {
			T data;
			Read(data);
			return data;
		}


		// Variable-Size size (1 or 9 bytes)
		INLINE void WriteSize(const size_t& size) {
			if (size < BYTE_MAX_VALUE) {
				Write((byte)size);
			} else {
				Write((byte)BYTE_MAX_VALUE);
				Write((size_t)size);
			}
		}
		INLINE size_t ReadSize() {
			size_t size = Read<byte>();
			if (size == BYTE_MAX_VALUE) {
				Read(size);
			}
			return size;
		}


		// Strings
		INLINE ByteStream& Write(const std::string& data) {
			WriteSize(data.size());
			WriteBytes(reinterpret_cast<const byte*>(data.c_str()), data.size());
			return *this;
		}
		INLINE ByteStream& Read(std::string& data) {
			size_t size(ReadSize());
			char chars[size];
			ReadBytes(reinterpret_cast<byte*>(chars), size);
			data.insert(0, chars, size);
			return *this;
		}


		// Containers (vector or other containers that have the following methods: .size(), .clear(), .reserve() and .push_back())
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& Write(const Container<T, std::allocator<T>>& data) {
			WriteSize(data.size());
			for (const T& item : data) Write(item);
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& Read(Container<T, std::allocator<T>>& data) {
			size_t size(ReadSize());
			data.clear();
			data.reserve(size);
			for (size_t i = 0; i < size; i++) data.push_back(Read<T>());
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE Container<T, std::allocator<T>> Read() {
			Container<T, std::allocator<T>> data;
			Read<Container, T>(data);
			return data;
		}


		// Stream Operators overloading (generic)
		template<typename T>
		INLINE ByteStream& operator<<(const T& data) {
			Write(data);
			return *this;
		}
		template<typename T>
		INLINE ByteStream& operator>>(T& data) {
			Read(data);
			return *this;
		}


		// Stream Operators overloading (containers)
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& operator<<(const Container<T, std::allocator<T>>& data) {
			Write<Container, T>(data);
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& operator>>(Container<T, std::allocator<T>>& data) {
			Read<Container, T>(data);
			return *this;
		}


		// Variadic templates
		template<typename ...Args>
		INLINE ByteStream& Write(const Args&... args) {
			return (..., this->Write<Args>(args));
		}
		template<typename ...Args>
		INLINE ByteStream& Read(Args&... args) {
			return (..., this->Read<Args>(args));
		}


	};
}
