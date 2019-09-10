#pragma once

#include <v4d.h>

namespace v4d {
	class Stream {
	protected:
		std::recursive_mutex writeMutex;
		std::vector<byte> writeBuffer;

	public:

		Stream(size_t writeBufferSize) {
			writeBuffer.reserve(writeBufferSize);
		}

		virtual ~Stream(){}

		virtual Stream& WriteBytes(const byte* data, const size_t& n) {
			std::lock_guard lock(writeMutex);
			if ((writeBuffer.size() + n) > writeBuffer.capacity()) {
				Flush();
			}
			writeBuffer.insert(writeBuffer.end(), data, data+n);
			return *this;
		}


		// Lock / Unlock
		INLINE void LockWriteBuffer() {
			writeMutex.lock();
		}
		INLINE void UnlockWriteBuffer() {
			writeMutex.unlock();
		}


		// Pure-Virtual methods (MUST override)
		virtual Stream& ReadBytes(byte* data, const size_t& n) = 0;
		virtual Stream& Flush() = 0;


		// Read/Write methods
		template<typename T>
		INLINE Stream& Write(const T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				std::lock_guard lock(writeMutex);
				WriteSize(data.size());
				return WriteBytes(reinterpret_cast<const byte*>(data.c_str()), data.size());
			} else {
				// Any other type
				return WriteBytes(reinterpret_cast<const byte*>(&data), sizeof(T));
			}
		}
		template<typename T>
		INLINE Stream& Read(T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				size_t size(ReadSize());
				char chars[size];
				ReadBytes(reinterpret_cast<byte*>(chars), size);
				data.insert(0, chars, size);
				return *this;
			} else {
				// Any other type
				return ReadBytes(reinterpret_cast<byte*>(&data), sizeof(T));
			}
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
			std::lock_guard lock(writeMutex);
			if (size < MAXBYTE) {
				Write((const byte)size);
			} else {
				Write((const byte)MAXBYTE);
				Write((const size_t)size);
			}
		}
		INLINE size_t ReadSize() {
			size_t size = Read<byte>();
			if (size == MAXBYTE) {
				Read(size);
			}
			return size;
		}


		// Containers (vector or other containers that have the following methods: .size(), .clear(), .reserve() and .push_back())
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& Write(const Container<T, std::allocator<T>>& data) {
			std::lock_guard lock(writeMutex);
			WriteSize(data.size());
			for (const T& item : data) Write(item);
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& Read(Container<T, std::allocator<T>>& data) {
			size_t size(ReadSize());
			data.clear();
			data.reserve(size);
			for (size_t i = 0; i < size; i++) data.push_back(Read<T>());
			return *this;
		}
		// Return-Read with container
		template<template<typename, typename> class Container, typename T>
		INLINE Container<T, std::allocator<T>> Read() {
			Container<T, std::allocator<T>> data;
			Read<Container, T>(data);
			return data;
		}


		// Stream Operators overloading (generic)
		template<typename T>
		INLINE Stream& operator<<(const T& data) {
			return Write((const T&)data);
		}
		template<typename T>
		INLINE Stream& operator>>(T& data) {
			return Read(data);
		}


		// Stream Operators overloading (containers)
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& operator<<(const Container<T, std::allocator<T>>& data) {
			return Write<Container, T>(data);
		}
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& operator>>(Container<T, std::allocator<T>>& data) {
			return Read<Container, T>(data);
		}


		// Variadic templates
		template<typename ...Args>
		INLINE Stream& Write(const Args&... args) {
			std::lock_guard lock(writeMutex);
			return (..., this->Write<const Args&>(args));
		}
		template<typename ...Args>
		INLINE Stream& Read(Args&... args) {
			return (..., this->Read<Args>(args));
		}


	};
}

