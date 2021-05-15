#pragma once

#include <v4d.h>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include "utilities/crypto/AES.h"

namespace v4d::networking {
	class V4DLIB IncomingClient {
	public:
		int32_t id;
		
		std::string token = "";
		std::unique_ptr<v4d::crypto::AES> aes = nullptr;
		std::atomic<uint64_t> requestIncrement = 0;
		
		IncomingClient(int32_t id);

		~IncomingClient();
		
		void EmplaceThread(std::function<void()>&& func) {
			std::lock_guard lock(threadsMutex);
			threads.emplace_back(std::forward<std::function<void()>>(func));
		}

		DELETE_COPY_MOVE_CONSTRUCTORS(IncomingClient)
		
	protected:
		std::mutex threadsMutex;
		std::vector<std::thread> threads {};
	};
	
	typedef std::shared_ptr<IncomingClient> IncomingClientPtr;
}
