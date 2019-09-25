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
	const byte	DENY	= 18;	// Denied
	const byte	OK  	= 19;	// Approved / Success

	// Client requests to Server
	const byte	CONN	= 24;	// Connect (+ APPNAME + VERSION + CLIENT_TYPE) (No response from server. Server may immediately close the connection)
	const byte	AUTH	= 25;	// Authenticate (+ rsaEncrypted_auth_data) (Server responds with OK+rsaSignature_of_decrypted_auth_data | DENY+errcode+errmsg)
	const byte	ENCRYPT	= 26;	// Send encryption key (+ rsaEncrypted_AES_KEY) (Server responds with aesEncrypted_token)

	// Server requests to client
	const byte	MSG 	= 29;	// Message (+ string) (No response from client). The client may display that message for the user to read.

	// Misc...
	const byte	HELLO	= 254;	// First byte upon connection for any communication from both sides. If not received, the connection may be closed after 500ms
	const byte	EXT 	= 255;	// Extended Requests (for app-specific request types)
}
