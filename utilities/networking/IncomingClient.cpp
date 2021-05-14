#include "IncomingClient.h"

using namespace v4d::networking;

IncomingClient::IncomingClient(int32_t id) : id(id) {}

IncomingClient::~IncomingClient() {
	std::lock_guard lock(threadsMutex);
	for (std::thread& t : threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}
