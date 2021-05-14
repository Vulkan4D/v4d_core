#pragma once

#include "IncomingClient.h"

namespace v4d::networking {

class V4DLIB ClientPool {
protected:
	virtual ~ClientPool(); // just for vTable
public:
	virtual IncomingClientPtr GetClient(uint32_t index) = 0;
	virtual IncomingClientPtr NewClient(uint32_t& index) = 0;
	virtual void DeleteClient(uint32_t index) = 0;
	virtual void Clear() = 0;
	virtual void Cleanup() = 0;
};

class V4DLIB BasicClientPool : public ClientPool {
	std::vector<IncomingClientPtr> clients {};
	std::mutex insertRemoveMutex;
	uint32_t nextClientIndex = 0;
public:
	BasicClientPool(size_t size) {clients.resize(size);}
	~BasicClientPool() = default;
	
	IncomingClientPtr GetClient(uint32_t index) override {
		if (index < clients.size()) return clients[index];
		return nullptr;
	}
	IncomingClientPtr NewClient(uint32_t& index) override {
		std::lock_guard insertRemoveLock(insertRemoveMutex);
		do {
			index = nextClientIndex++;
			assert(index < clients.size());
		} while (clients[index] != nullptr);
		return clients[index] = std::make_shared<IncomingClient>(index);
	}
	void DeleteClient(uint32_t index) override {
		if (index < clients.size()) {
			std::lock_guard insertRemoveLock(insertRemoveMutex);
			clients[index] = nullptr;
			nextClientIndex = index;
		}
	}
	void Clear() override {
		std::lock_guard insertRemoveLock(insertRemoveMutex);
		for (auto& c : clients) {
			c.reset();
		}
		nextClientIndex = 0;
	}
	void Cleanup() override {}
};
}
