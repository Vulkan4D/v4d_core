#pragma once

#include <v4d.h>
#include <vector>
#include <functional>
#include <mutex>

#include "utilities/io/Logger.h"
#include "utilities/crypto/Crypto.h"

namespace v4d::data {

	class ReadOnlyStream;

	class V4DLIB Stream {

	private: // members

		// Optional Read Buffer
		bool useReadBuffer = false;
		size_t readBufferCursor = 0;
		
		std::vector<byte> readBuffer{};
		std::vector<byte> writeBuffer{};
		
		std::recursive_mutex writeMutex, readMutex;
		
	public: // optional Begin/End lambdas for safe and flexible usage when passing socket ptr to a module or function for it to send streams
		std::function<void()> Begin = [](){};
		std::function<void()> End = [](){};

	protected: // Virtual methods to override for data sources

		virtual void Send() {}
		virtual size_t Receive(byte*, size_t) {return 0;}

		virtual void ReadBytes_OnError(const char*) {}


	public: // Utility methods

		// Lock / Unlock
		void LockWrite() {
			writeMutex.lock();
		}
		void UnlockWrite() {
			writeMutex.unlock();
		}
		void LockRead() {
			readMutex.lock();
		}
		void UnlockRead() {
			readMutex.unlock();
		}
		void LockReadWrite() {
			readMutex.lock();
			writeMutex.lock();
		}
		void UnlockReadWrite() {
			writeMutex.unlock();
			readMutex.unlock();
		}

		// Buffer access
		size_t GetReadBufferSize() {
			std::lock_guard lock(readMutex);
			return readBuffer.size();
		}
		size_t GetWriteBufferSize() {
			std::lock_guard lock(readMutex);
			return writeBuffer.size();
		}
		void ClearReadBuffer() {
			std::lock_guard lock(readMutex);
			readBuffer.resize(0);
		}
		void ClearWriteBuffer() {
			std::lock_guard lock(writeMutex);
			writeBuffer.resize(0);
		}
		virtual std::vector<byte>& _GetReadBuffer_() {
			return readBuffer;
		}
		virtual std::vector<byte>& _GetWriteBuffer_() {
			return writeBuffer;
		}

		virtual std::vector<byte> GetData() {
			std::lock_guard lock(writeMutex);
			// Copy and return buffer
			return writeBuffer;
		}

	public: // Constructor & Destructor

		Stream(size_t bufferSize = 512, bool useReadBuffer = false) : useReadBuffer(useReadBuffer), readBufferCursor(0) {
			if (bufferSize > 0) {
				writeBuffer.reserve(bufferSize);
			}
			if (useReadBuffer && bufferSize > 0) {
				readBuffer.reserve(bufferSize);
			}
		}
		
		Stream(const Stream& stream) : useReadBuffer(stream.useReadBuffer), readBufferCursor(0) {
			writeBuffer = stream.writeBuffer;
			if (useReadBuffer) readBuffer = stream.readBuffer;
		}

		virtual ~Stream() {}

		DELETE_MOVE_CONSTRUCTORS(Stream)


	public: // Read & Write

		// Flush
		virtual Stream& Flush() {
			std::lock_guard lock(writeMutex);
			Send();
			writeBuffer.resize(0);
			return *this;
		}
		// virtual Stream& FlushDebug() {
		// 	std::lock_guard lock(writeMutex);
		// 	DEBUG("Debug Flush " << writeBuffer.size() << " " << writeBuffer)
		// 	return Flush();
		// }

		void ResetReadBuffer() {
			std::lock_guard lock(readMutex);
			readBuffer.resize(0);
			readBufferCursor = 0;
		}

		virtual Stream& WriteBytes(const byte* data, size_t n);
		virtual Stream& ReadBytes(byte* data, size_t n);

	public: // Read & Write (Overloads & Templates)

		template<typename T>
		Stream& Write(const T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				std::lock_guard lock(writeMutex);
				WriteSize(data.size());
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					return WriteBytes(reinterpret_cast<const byte*>(data.c_str()), data.size());
				#else
					if (data.size() > 0) {
						byte bytes[data.size()];
						memcpy(bytes, data.c_str(), data.size());
						return WriteBytes(bytes, data.size());
					} else {
						return *this;
					}
				#endif
			} else {
				// Any other type
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					return WriteBytes(reinterpret_cast<const byte*>(&data), sizeof(T));
				#else
					byte bytes[sizeof(T)];
					memcpy(bytes, &data, sizeof(T));
					return WriteBytes(bytes, sizeof(T));
				#endif
			}
		}
		template<typename T>
		Stream& Read(T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				std::lock_guard lock(readMutex);
				size_t size(ReadSize());
				if (size <= 0) {
					data = "";
					return *this;
				}
				byte bytes[size];
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					ReadBytes(bytes, size);
					data = std::string(reinterpret_cast<char const*>(bytes), size);
				#else
					char chars[size + 1];
					ReadBytes(bytes, size);
					memcpy(chars, bytes, size);
					chars[size] = 0;
					data = chars;
				#endif
				return *this;
			} else {
				// Any other type
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					return ReadBytes(reinterpret_cast<byte*>(&data), sizeof(T));
				#else
					byte bytes[sizeof(T)];
					ReadBytes(bytes, sizeof(T));
					memcpy(&data, bytes, sizeof(T));
					return *this;
				#endif
			}
		}


		// // Return-Read
		// template<typename T>
		// T Read() {
		// 	T data{};
		// 	Read<T>(data);
		// 	return data;
		// }


		// Return-Read
		template<typename T>
		T Read() {
			if constexpr (std::is_same_v<T, std::string>) {
				T data = "";
				Read(data);
				return data;
			} else if constexpr (std::is_arithmetic_v<T>) {
				T data = 0;
				Read(data);
				return data;
			} else {
				T data{};
				Read(data);
				return data;
			}
		}



		// Variable-Size size (1 or 9 bytes)
		void WriteSize(size_t size) {
			std::lock_guard lock(writeMutex);
			if (size < MAXBYTE) {
				Write<byte>((byte)size);
			} else {
				Write<byte>((byte)MAXBYTE);
				Write<size_t>(size);
			}
		}
		size_t ReadSize() {
			std::lock_guard lock(readMutex);
			size_t size = Read<byte>();
			if (size == MAXBYTE) {
				Read<size_t>(size);
			}
			return size;
		}


		// Containers (vector or other containers that have the following methods: .size(), .clear(), .reserve() and .push_back())
		template<template<typename, typename> class Container, typename T>
		Stream& Write(const Container<T, std::allocator<T>>& data) {
			std::lock_guard lock(writeMutex);
			WriteSize(data.size());
			for (const T& item : data) Write<T>(item);
			return *this;
		}
		template<template<typename, typename> class Container, typename T>
		Stream& Read(Container<T, std::allocator<T>>& data) {
			std::lock_guard lock(readMutex);
			size_t size{ReadSize()};
			data.clear();
			data.reserve(size);
			for (size_t i = 0; i < size; i++) data.push_back(Read<T>());
			return *this;
		}
		// Return-Read with container
		template<template<typename, typename> class Container, typename T>
		Container<T, std::allocator<T>> Read() {
			Container<T, std::allocator<T>> data{};
			Read<Container, T>(data);
			return data;
		}


		// Stream Operators overloading (generic)
		template<typename T>
		Stream& operator<<(const T& data) {
			return Write((const T&)data);
		}
		template<typename T>
		Stream& operator>>(T& data) {
			return Read(data);
		}


		// Stream Operators overloading (containers)
		template<template<typename, typename> class Container, typename T>
		Stream& operator<<(const Container<T, std::allocator<T>>& data) {
			return Write<Container, T>(data);
		}
		template<template<typename, typename> class Container, typename T>
		Stream& operator>>(Container<T, std::allocator<T>>& data) {
			return Read<Container, T>(data);
		}


		// Variadic templates
		template<typename ...Args>
		Stream& Write(const Args&... args) {
			std::lock_guard lock(writeMutex);
			return (..., this->Write<const Args&>(args));
		}
		template<typename ...Args>
		Stream& Read(Args&... args) {
			std::lock_guard lock(readMutex);
			return (..., this->Read<Args>(args));
		}


		////////////////////////////////////////////////////////////////////////////////
		// Other Read/Write methods

		// Stream
		ReadOnlyStream ReadStream();
		void ReadStream(ReadOnlyStream& stream);
		void WriteStream(Stream& stream);
		void EmplaceStream(Stream& stream);

		// Encryption
		template<typename T>
		Stream& WriteEncrypted(v4d::crypto::Crypto* crypto, const T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				return Write<std::vector, byte>(crypto->EncryptString(data));
			} else {
				// Any other type
				#ifdef ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
					return Write<std::vector, byte>(crypto->Encrypt(reinterpret_cast<const byte*>(&data), sizeof(T)));
				#else
					byte bytes[sizeof(T)];
					memcpy(bytes, &data, sizeof(T));
					return Write<std::vector, byte>(crypto->Encrypt(bytes, sizeof(T)));
				#endif
			}
		}
		template<typename T>
		Stream& ReadEncrypted(v4d::crypto::Crypto* crypto, T& data) {
			if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				data = crypto->DecryptString(Read<std::vector, byte>());
				return *this;
			} else {
				// Any other type
				auto decrypted = crypto->Decrypt(Read<std::vector, byte>());
				memcpy(&data, decrypted.data(), sizeof(T));
				return *this;
			}
		}

		// Return-Read Encrypted
		template<typename T>
		T ReadEncrypted(v4d::crypto::Crypto* crypto) {
			T data{};
			ReadEncrypted(crypto, data);
			return data;
		}

		// Encrypted Stream
		ReadOnlyStream ReadEncryptedStream(v4d::crypto::Crypto* crypto);
		void WriteEncryptedStream(v4d::crypto::Crypto* crypto, Stream& stream);

	};
}

