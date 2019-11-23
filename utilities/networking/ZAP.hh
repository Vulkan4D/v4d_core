#pragma once
#include <v4d.h>

// ZAP Ain't a Protocol
namespace v4d::networking::ZAP {
	// Error, disconnect now
	const byte	ERR 	= 0;	// An error occured

	// One of these must be at the end of any transmission made, at least during the handshake process.
	const byte	WAIT	= 2;	// Wait indefinitely for more requests
	const byte	OVER	= 3;	// There will be no more requests for now, awaiting response or request
	const byte	BYE 	= 4;	// Disconnected

	// Ping/Pong (request/response from anyone)
	const byte	PING	= 7;	// Ping request from either
	const byte	PONG	= 8;	// Pong response from either

	// Basic responses
	const byte	ACK 	= 17;	// Acknowledged previous request
	const byte	DENY	= 18;	// Denied / Failed
	const byte	OK  	= 19;	// Approved / Success

	// Client requests to server
	const byte 	PUBKEY	= 21; // Client asks server to send its public key (RSA)

	// AUTHTYPE Client to Server (Each connection requires sending HELLO + APPNAME + VERSION + CLIENT_TYPE + AUTHTYPE)
	const byte	AUTH	= 24;	// Connect with AUTH (+ rsaEncrypted_auth_data + rsaEncrypted_AES_KEY) Server response: OK+rsaSignature_of_decrypted_auth_data+aesEncrypted_token_and_id | DENY+errcode+errmsg
	const byte	TOKEN	= 25;	// Connect with Token (+ (ulong)id + aesEncrypted_token) Server response: OK | DENY+errcode+errmsg
	const byte	ANONYM	= 26;	// Connect anonymously ... Server response: OK | DENY+errcode+errmsg

	// Server requests to client
	const byte	MSG 	= 29;	// Message (+ string) (No response from client). The client may display that message for the user to read.

	// Misc...
	const byte	HELLO	= 254;	// First byte upon connection for any communication from both sides. If not received, the connection may be closed after 100ms
	const byte	EXT 	= 255;	// Extended Requests (for application-specific request types)


	/////////////////////////////////////////////////////////////
	// Typical connection request/handshake
	/*
	Client									<-->	Server
	------------------------------------------------------
	Get Public Key : 
		ClientHello								---> 
		PUBKEY (Byte)							--->
												<---	OK (Byte)
												<---	PublicKey (String PEM)
														Disconnect
	------------------------------------------------------
	AUTH connection : 
		ClientHello								---> 
		AUTH (Byte)								--->
		AuthDataAndAesKey (EncryptedStream) rsa	--->	(server decrypts and confirms auth data)
												<---	OK (Byte)
												<---	Signature Of AuthDataAndAesKey (Vector<Byte>) rsa
												<---	tokenAndId (EncryptedStream) aes
														Stay connected
	------------------------------------------------------
	TOKEN connection : 
		ClientHello								---> 
		TOKEN (Byte)							--->
		ClientToken aesEncrypted				--->	(server decrypts and checks id, token and increment>lastIncrement)
												<---	OK (Byte)
														Stay connected
	*/


	/////////////////////////////////////////////////////////////
	// Data structures

	namespace __reflection {

		// SFINAE for STL Containers... check for a method like size()
		template <typename T>
		class IsSTLContainer {
			typedef std::true_type yes;
			typedef std::false_type no;
			template<typename U> static auto test(int) -> decltype(std::declval<U>().size() == 1, yes());
			template<typename> static no test(...);
			public: static constexpr bool value = std::is_same_v<decltype(test<T>(0)),yes>;
		};

		// ZAP Data Write
		template <typename T>
		class __StructHasWriteMethod {
			typedef std::true_type yes;
			typedef std::false_type no;
			template <typename classType> static yes& test(decltype(&classType::Write)) ;
			template <typename classType> static no& test(...);
			public: static constexpr bool value = std::is_same_v<decltype(test<T>(0)),yes>;
		};
		template <typename T>
		void __ForEachDataMemberWrite(v4d::data::Stream* stream, const T& t) {
			if constexpr (std::is_arithmetic_v<T>) {
				// Int, Bool, Byte, Float...
				*stream << t;
			} else if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				*stream << t;
			} else if constexpr (IsSTLContainer<T>::value) {
				// vector<int> ...
				*stream << t;
			} else if constexpr (__StructHasWriteMethod<T>::value) {
				// ZAPDATA custom Write method
				t.Write(stream);
			} else {
				// Any other Struct (ZAPDATA)
				std::apply([stream](auto&&... args) {
					stream->LockWrite();
					(__ForEachDataMemberWrite(stream, args),...);
					stream->UnlockWrite();
				}, ReflMemberGroup(t));
			}
		}

		// ZAP Data Read
		template <typename T>
		class __StructHasReadMethod {
			typedef char yes[1]; typedef char no[2];
			template <typename classType> static yes& test(decltype(&classType::Read)) ;
			template <typename classType> static no& test(...);
			public: enum {value = (sizeof(test<T>(0)) == sizeof(yes))};
		};
		template <typename T>
		void __ForEachDataMemberRead(v4d::data::Stream* stream, T& t) {
			if constexpr (std::is_arithmetic_v<T>) {
				// Int, Bool, Byte, Float...
				*stream >> t;
			} else if constexpr (std::is_same_v<T, std::string>) {
				// std::string
				*stream >> t;
			} else if constexpr (IsSTLContainer<T>::value) {
				// vector<int> ...
				*stream >> t;
			} else if constexpr (__StructHasReadMethod<T>::value) {
				// ZAPDATA custom Read method
				t.Read(stream);
			} else {
				// Any other Struct (ZAPDATA)
				std::apply([stream](auto&&... args) {
					stream->LockRead();
					(__ForEachDataMemberRead(stream, args),...);
					stream->UnlockRead();
				}, ReflMemberGroup(t));
			}
		}

		// ZAP Data MACRO
		#define ZAPABLE(structName) \
		friend v4d::data::Stream& operator<<(v4d::data::Stream& stream, const structName& obj) { \
			__reflection::__ForEachDataMemberWrite(&stream, obj); \
			return stream; \
		} friend v4d::data::Stream& operator>>(v4d::data::Stream& stream, structName& obj) { \
			__reflection::__ForEachDataMemberRead(&stream, obj); \
			return stream; \
		} \
		static structName ReadFrom(v4d::data::Stream& stream) { \
			structName data; \
			stream >> data; \
			return data; \
		} \
		static structName ReadFrom(v4d::data::Stream* stream) { \
			structName data; \
			*stream >> data; \
			return data; \
		} \
		static structName ReadFrom(v4d::io::SharedSocket& stream) { \
			structName data; \
			*stream >> data; \
			return data; \
		}
		#define ZAPDATA(structName, body) struct structName { ZAPABLE(structName) body ; };
	}

	namespace data {

		// Arythmetic types
		using Char = char;
		using Byte = byte; // unsigned char
		using Int16 = int16_t;
		using Int16_u = uint16_t;
		using Int32 = int32_t;
		using Int32_u = uint32_t;
		using Int64 = int64_t;
		using Int64_u = uint64_t;
		using Float32 = float;
		using Float64 = double;
		using Bool = bool;

		// Complex types
		using String = std::string;
		template<typename T> using Vector = std::vector<T>;
		using Bytes = Vector<Byte>;

		// Encrypted data structures
		ZAPDATA( EncryptedStream, 
			Bytes encryptedData;
			EncryptedStream& Encrypt(v4d::crypto::Crypto* crypto, v4d::data::Stream& stream) {
				encryptedData = crypto->EncryptStream(stream);
				return *this;
			}
			v4d::data::ReadOnlyStream Decrypt(v4d::crypto::Crypto* crypto) {
				return crypto->DecryptStream(encryptedData);
			}
		)
		ZAPDATA( EncryptedString, 
			Bytes encryptedData;
			EncryptedString& Encrypt(v4d::crypto::Crypto* crypto, String& str) {
				encryptedData = crypto->EncryptString(str);
				return *this;
			}
			String Decrypt(v4d::crypto::Crypto* crypto) {
				return crypto->DecryptString(encryptedData);
			}
		)

		// Handshake
		ZAPDATA( ClientHello, 
			String appName;
			String version;
			Byte clientType;
		)
		ZAPDATA( ClientToken, 
			Int64_u id;
			Int64_u increment;
			EncryptedString token;
		)
		ZAPDATA( Error, 
			Int32 code;
			std::string message;
		)


		/* Additional data structure example. 
			Limitations: 
				Cannot inherit from another struct/class
				Cannot define any constructors
				Members can only include Generic types or other ZAPDATA types.
				Cannot have a comma in the body. If we need a comma somewhere, create a normal struct and use ZAPABLE(className) macro inside the body instead.

		ZAPDATA( Test, 
			// basic data
			Byte action;
			Int32 index;
			Bool isActive;
			// more complex data
			String someText; // Normal string, variable in length
			Vector<Float32> variableVectorOfFloats; // Variable-size vectors of any of these data structs are possible too
			// Custom Read/Write methods (optional, if we need to read or write the data in a very specific way)
			void Read(v4d::data::Stream* stream) {
				*stream 
					>> action 
					>> index 
					>> isActive 
					>> someText 
					>> variableVectorOfFloats
				;
			}
			void Write(v4d::data::Stream* stream) const {
				*stream 
					<< action 
					<< index 
					<< isActive 
					<< someText 
					<< variableVectorOfFloats
				;
			}
		)
		*/


	}
}


/////////////////////////////////////////////////////////////
// Namespace Aliases
namespace v4dnet = ::v4d::networking;
namespace zap = ::v4d::networking::ZAP;
namespace zapdata = ::v4d::networking::ZAP::data;


/////////////////////////////////////////////////////////////
// ZAP Codes

#define DEFINE_ZAP_CODE(code, name, msg) namespace v4d::networking::ZAP_CODES { const int name = code; const std::string name ## _text = msg; }

DEFINE_ZAP_CODE(101, DENY_ANONYMOUS, "Cannot connect anonymously to this server")
DEFINE_ZAP_CODE(102, APPNAME_MISMATCH, "App incompatible with server")
DEFINE_ZAP_CODE(103, VERSION_MISMATCH, "App version does not match server version")
DEFINE_ZAP_CODE(104, AUTH_FAILED, "Authentication failed")
DEFINE_ZAP_CODE(105, AUTH_FAILED_INVALID_ID, "Authentication failed: invalid ID")
DEFINE_ZAP_CODE(106, AUTH_FAILED_INVALID_TOKEN, "Authentication failed: invalid TOKEN")
DEFINE_ZAP_CODE(107, AUTH_FAILED_HANDSHAKE, "Authentication failed: handshake failed")
DEFINE_ZAP_CODE(108, AUTH_FAILED_RSA_DECRYPTION, "Authentication failed: Data not encrypted correctly")
DEFINE_ZAP_CODE(109, SERVER_RESPONSE_TIMEOUT, "Server did not respond in time")
DEFINE_ZAP_CODE(110, SERVER_SIGNATURE_FAILED, "Server authenticity check has failed: Invalid Signature")
DEFINE_ZAP_CODE(111, AUTH_FAILED_REQ_INCREMENT, "Authentication failed: Invalid Request Increment")

// For definitions of application-specific ZAP CODES, use integers above 1000
