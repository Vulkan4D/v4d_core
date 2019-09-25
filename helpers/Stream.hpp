#pragma once

#include <v4d.h>

namespace v4d {
	class Stream {

	private: // members

		// Optional Read Buffer
		bool useReadBuffer = false;
		size_t readBufferCursor;
		std::vector<byte> readBuffer;

		std::vector<byte> writeBuffer;


	public: // members

		std::recursive_mutex writeMutex, readMutex;


	protected: // Virtual methods to override for data sources

		virtual void Send() {}
		virtual size_t Receive(byte*, size_t) {return 0;}

		virtual void ReadBytes_OnError(const char*) {}


	public: // Utility methods

		// Lock / Unlock
		INLINE void LockWrite() {
			writeMutex.lock();
		}
		INLINE void UnlockWrite() {
			writeMutex.unlock();
		}
		INLINE void LockRead() {
			readMutex.lock();
		}
		INLINE void UnlockRead() {
			readMutex.unlock();
		}

		// Buffer access
		INLINE size_t GetReadBufferSize() {
			std::lock_guard lock(readMutex);
			return readBuffer.size();
		}
		INLINE size_t GetWriteBufferSize() {
			std::lock_guard lock(readMutex);
			return readBuffer.size();
		}
		INLINE void ClearReadBuffer() {
			std::lock_guard lock(readMutex);
			readBuffer.resize(0);
		}
		INLINE void ClearWriteBuffer() {
			std::lock_guard lock(writeMutex);
			writeBuffer.resize(0);
		}
		INLINE std::vector<byte>& _GetReadBuffer_() {
			return readBuffer;
		}
		INLINE std::vector<byte>& _GetWriteBuffer_() {
			return writeBuffer;
		}


	public: // Constructor & Destructor

		Stream(size_t bufferSize, bool useReadBuffer = false) : useReadBuffer(useReadBuffer), readBufferCursor(0) {
			if (bufferSize > 0) {
				writeBuffer.reserve(bufferSize);
			}
			if (useReadBuffer) {
				readBuffer.reserve(bufferSize);
			}
		}

		virtual ~Stream(){}


	public: // Read & Write

		// Flush
		virtual Stream& Flush() {
			std::lock_guard lock(writeMutex);
			Send();
			writeBuffer.resize(0);
			return *this;
		}

		INLINE void ResetReadBuffer() {
			std::lock_guard lock(readMutex);
			readBuffer.resize(0);
			readBufferCursor = 0;
		}

		virtual Stream& WriteBytes(const byte* data, size_t n) {
			std::lock_guard lock(writeMutex);
			if ((writeBuffer.size() + n) > writeBuffer.capacity()) {
				Flush();
			}
			writeBuffer.insert(writeBuffer.end(), data, data+n);
			return *this;
		}

		virtual Stream& ReadBytes(byte* data, size_t n) {
			std::lock_guard lock(readMutex);
			if (useReadBuffer) {
				// Reset read buffer if we have read it completely
				if (readBufferCursor == readBuffer.size()) {
					ResetReadBuffer();
				}
				size_t remainingDataInBuffer = readBuffer.size() - readBufferCursor;
				// If we are missing data from buffer, fetch more from source
				if (remainingDataInBuffer < n) {
					size_t remainingDataToReceive = n - remainingDataInBuffer;
					size_t maxReadSize = readBuffer.capacity() - readBufferCursor;
					// If we dont have enough space left in our buffer, remove elements that are already read
					if (maxReadSize < remainingDataToReceive) {
						if (readBufferCursor == readBuffer.size()) {
							// If we were finished with the buffer, we should reset it
							maxReadSize = readBuffer.capacity();
						} else {
							// Erase only the part that we already read from the buffer
							readBuffer.erase(readBuffer.begin(), readBuffer.begin() + (long)readBufferCursor);
							maxReadSize = readBuffer.capacity() - readBuffer.size();
						}
						readBufferCursor = 0;
						// If we still dont have enough space left in our buffer, ERROR
						if (maxReadSize < remainingDataToReceive) {
							ReadBytes_OnError("Stream ReadBuffer capacity exceeded");
							ResetReadBuffer();
							std::memset(data, 0, n);
							return *this;
						}
					}
					// temporary resize the buffer to fit new data
					readBuffer.resize(readBufferCursor + maxReadSize);
					// Actually receive the new data (and put it directly in the buffer)
					size_t bytesRead = Receive(readBuffer.data(), maxReadSize);
					if (bytesRead == 0) {
						ReadBytes_OnError("Stream ReadBuffer received 0 bytes");
						ResetReadBuffer();
						std::memset(data, 0, n);
						return *this;
					}
					// Resize the buffer to its final size to include the new data received
					if (bytesRead > maxReadSize) {
						ReadBytes_OnError("Stream Receive bytesRead exceeded maxReadSize");
						ResetReadBuffer();
						std::memset(data, 0, n);
						return *this;
					} else if (bytesRead != maxReadSize) {
						readBuffer.resize(readBufferCursor + bytesRead);
					}
				}
				std::memcpy(data, readBuffer.data() + readBufferCursor, n);
				readBufferCursor += n;
			} else {
				Receive(data, n);
			}
			return *this;
		}


	public: // Read & Write (Overloads & Templates)

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
				std::lock_guard lock(readMutex);
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
			T data{};
			Read<T>(data);
			return data;
		}


		// Variable-Size size (1 or 9 bytes)
		INLINE void WriteSize(size_t size) {
			std::lock_guard lock(writeMutex);
			if (size < MAXBYTE) {
				Write<byte>((byte)size);
			} else {
				Write<byte>((byte)MAXBYTE);
				Write<size_t>(size);
			}
		}
		INLINE size_t ReadSize() {
			std::lock_guard lock(readMutex);
			size_t size = Read<byte>();
			if (size == MAXBYTE) {
				Read<size_t>(size);
			}
			return size;
		}


		// Containers (vector or other containers that have the following methods: .size(), .clear(), .reserve() and .push_back())
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& Write(const Container<T, std::allocator<T>>& data) {
			std::lock_guard lock(writeMutex);
			WriteSize(data.size());
			for (const T& item : data) Write<T>(item);
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		INLINE Stream& Read(Container<T, std::allocator<T>>& data) {
			std::lock_guard lock(readMutex);
			size_t size{ReadSize()};
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
			return Write<T>((const T&)data);
		}
		template<typename T>
		INLINE Stream& operator>>(T& data) {
			return Read<T>(data);
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
			std::lock_guard lock(readMutex);
			return (..., this->Read<Args>(args));
		}


	};
}

