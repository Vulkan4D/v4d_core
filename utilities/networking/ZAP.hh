#pragma once

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

	// AUTHTYPE Client to Server (Each connection requires sending HELLO + APPNAME + VERSION + CLIENT_TYPE + AUTHTYPE)
	const byte	AUTH	= 24;	// Connect with AUTH (+ rsaEncrypted_auth_data + rsaEncrypted_AES_KEY) Server response: OK+rsaSignature_of_decrypted_auth_data+aesEncrypted_token_and_id | DENY+errcode+errmsg
	const byte	TOKEN	= 25;	// Connect with Token (+ (ulong)id + aesEncrypted_token) Server response: OK | DENY+errcode+errmsg
	const byte	ANONYM	= 26;	// Connect anonymously ... Server response: OK | DENY+errcode+errmsg

	// Server requests to client
	const byte	MSG 	= 29;	// Message (+ string) (No response from client). The client may display that message for the user to read.

	// Misc...
	const byte	HELLO	= 254;	// First byte upon connection for any communication from both sides. If not received, the connection may be closed after 100ms
	const byte	EXT 	= 255;	// Extended Requests (for application-specific request types)
}

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

// For definitions of application-specific ZAP CODES, use integers above 1000
