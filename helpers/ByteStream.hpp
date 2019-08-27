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

		INLINE virtual void WriteBytes(const byte* data, size_t n) {
			std::lock_guard lock(writeMutex);
			if ((writeBuffer.size() + n) > writeBuffer.capacity()) {
				Flush();
			}
			writeBuffer.insert(writeBuffer.end(), data, data+n);
		}

		INLINE virtual void ReadBytes(byte* data, size_t n) {
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
		template<typename T>
		INLINE T Read() {
			T data;
			Read<T>(data);
			return data;
		}


		// Lists
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& Write(const Container<T, std::allocator<T>>& data) {
			*this << (size_t)data.size();
			for (const T& item : data) *this << (T)item;
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE ByteStream& Read(Container<T, std::allocator<T>>& data) {
			size_t size = Read<size_t>();
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


		// Operators overloading
		template<typename T>
		INLINE ByteStream& operator<<(const T& data) {
			Write<T>(data);
			return *this;
		}
		template<typename T>
		INLINE ByteStream& operator>>(T& data) {
			Read<T>(data);
			return *this;
		}
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
