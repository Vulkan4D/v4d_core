#include <v4d.h>

using namespace v4d::networking;

IncomingClient::IncomingClient(ulong id, std::string token, std::string aesHex) : id(id), token(token), aes(aesHex) {}
IncomingClient::IncomingClient(ulong id) : id(id), token(""), aes(256) {}

IncomingClient::~IncomingClient() {
	for (std::thread& t : threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}
