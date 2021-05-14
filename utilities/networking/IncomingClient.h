#pragma once

#include <v4d.h>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include "utilities/crypto/AES.h"

namespace v4d::networking {
	class IncomingClient {
	public:
		uint64_t id;
		std::string token;
		v4d::crypto::AES aes;
		std::vector<std::thread> threads{};
		std::atomic<uint64_t> requestIncrement = 0;
		uint8_t flags = 0; // reserved for use by application
		
		IncomingClient(uint64_t id, std::string token, std::string aesHex);
		IncomingClient(uint64_t id);

		~IncomingClient();

		DELETE_COPY_MOVE_CONSTRUCTORS(IncomingClient)
	};
	
	typedef std::shared_ptr<IncomingClient> IncomingClientPtr;
}
