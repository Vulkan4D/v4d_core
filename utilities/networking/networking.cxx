#include "utilities/networking/OutgoingConnection.h"
#include "utilities/networking/IncomingClient.h"
#include "utilities/networking/ListeningServer.h"
#include "utilities/io/Logger.h"

std::atomic<int> result = 100;

namespace v4d::networking::ZAP::data {
	struct Auth { STREAMABLE(Auth, username, password, stuff)
		String username;
		String password;
		Vector<Int32> stuff;
	};
}

class TestServer : public v4d::networking::ListeningServer{
public:
	using ListeningServer::ListeningServer;

	uint64_t GetAppName() const override {
		return v4d::BaseN::EncodeStringToUInt64("V4D", v4d::BASE40_UPPER_CHARS);
	}
	uint16_t GetVersion() const override {
		return 168;
	}

	v4d::networking::IncomingClientPtr Authenticate(v4d::data::ReadOnlyStream* encryptedStream, v4d::data::ReadOnlyStream* plainStream) override {
		if (encryptedStream) {
			auto[username, password, stuff] = zapdata::Auth::ConstructFromStream(encryptedStream);
			if (username == "bob" && password == "12345") {
				if (stuff.size() == 3 && stuff[0] == 4 && stuff[1] == 16 && stuff[2] == 512) {
					uint32_t id;
					return clientPool->NewClient(id);
				} else {
					return nullptr;
				}
			} else {
				return nullptr;
			}
		} else {
			// Anonymous
			return nullptr;
		}
	}

	void Communicate(v4d::io::SocketPtr, std::shared_ptr<v4d::networking::IncomingClient>, byte /*clientType*/) override {
		result -= 30;
	}

};

class TestClient : public v4d::networking::OutgoingConnection {
public:
	using OutgoingConnection::OutgoingConnection;

	uint64_t GetAppName() const override {
		return v4d::BaseN::EncodeStringToUInt64("V4D", v4d::BASE40_UPPER_CHARS);
	}
	uint16_t GetVersion() const override {
		return 168;
	}

	void Authenticate(v4d::data::Stream* encryptedStream, v4d::data::Stream* plainStream) override {
		*encryptedStream << zapdata::Auth{"bob", "12345", {4,16,512}};
	}
	
	void Communicate(v4d::io::SocketPtr /*socket*/) override {
		result -= 20;
	}

};

namespace v4d::tests {
	int Networking() {

		auto rsa = std::make_shared<v4d::crypto::RSA>(2048, 3);
		
		auto clientPool = std::make_shared<v4d::networking::BasicClientPool>(32);

		{// Test1
			// Server
			TestServer server(clientPool, v4d::io::TCP, rsa);
			server.Start(44444);

			auto rsaPublicKey = std::make_shared<v4d::crypto::RSA>(TestClient{v4d::io::TCP}.GetServerPublicKey("127.0.0.1", 44444), false);

			// Client
			TestClient client(v4d::io::TCP, rsaPublicKey);
			if (client.Connect("127.0.0.1", 44444, 1)) {
				SLEEP(100ms) // wait for server to have the time to decrement result
				client.Disconnect();
				if (result != 50) {
					LOG_ERROR(result << "  Networking Error Test1: Wrong result after first connect")
					return 4;
				}

				TestClient client2(&client);
				if (client2.Connect("127.0.0.1", 44444, 2)) {
					// SUCCESS
					SLEEP(100ms) // wait for server to have the time to decrement result
					if (result != 0) {
						LOG_ERROR(result << "  Networking Error Test1: Wrong result after second connect")
					}
					return result;// If fails without error, then final result is not good.
				} else {
					LOG_ERROR("Networking Error Test1: TOKEN Connection failed")
					return 2;
				}


			} else {
				LOG_ERROR("Networking Error Test1: AUTH Connection failed")
				return 1;
			}
		}

		return -1;
	}
}
