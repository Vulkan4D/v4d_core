#pragma once

#include <v4d.h>

namespace v4d::networking {
	class IncomingClient {
	public:
		ulong id;
		std::string token;
		v4d::crypto::AES aes;
		std::vector<std::thread> threads{};
		std::atomic<ulong> requestIncrement = 0;
		IncomingClient(ulong id, std::string token, std::string aesHex) : id(id), token(token), aes(aesHex) {}
		IncomingClient(ulong id) : id(id), token(""), aes(256) {}
		~IncomingClient() {
			for (std::thread& t : threads) if (t.joinable()) t.join();
		}
		DELETE_COPY_MOVE_CONSTRUCTORS(IncomingClient)
	};
}
