#include <v4d.h>

std::atomic<int> result = 100;

namespace v4d::networking::ZAP::data {
	ZAPDATA( Auth, 
		String username;
		String password;
		Vector<Int32> stuff;
	)
}

class TestServer : public v4d::networking::ListeningServer{
public:
	using ListeningServer::ListeningServer;

	std::string GetAppName() const override {
		return "V4D";
	}
	std::string GetVersion() const override {
		return "0.0.0";
	}

	ulong Authenticate(v4d::data::ReadOnlyStream* authStream) override {
		if (authStream) {
			auto[username, password, stuff] = zapdata::Auth::ReadFrom(authStream);
			if (username == "bob" && password == "12345") {
				if (stuff.size() == 3 && stuff[0] == 4 && stuff[1] == 16 && stuff[2] == 512) {
					return 16488; // ID
				} else {
					return 0;
				}
			} else {
				return 0;
			}
		} else {
			// Anonymous
			return 0;
		}
	}

	void RunClient(v4d::io::SharedSocket, std::shared_ptr<v4d::networking::IncomingClient>, byte /*clientType*/) override {
		result -= 30;
	}

};

class TestClient : public v4d::networking::OutgoingConnection {
public:
	using OutgoingConnection::OutgoingConnection;

	std::string GetAppName() const override {
		return "V4D";
	}
	std::string GetVersion() const override {
		return "0.0.0";
	}

	void Authenticate(v4d::data::Stream* authStream) override {
		*authStream << zapdata::Auth{"bob", "12345", {4,16,512}};
	}
	
	void Run(v4d::io::Socket& /*socket*/) override {
		result -= 20;
	}

};

namespace v4d::tests {
	int Networking() {

		// LOGGER_INSTANCE->SetVerbose(true);

		v4d::crypto::RSA rsa(2048, 3);

		LOG("Parent thread " << std::this_thread::get_id())

		{// Test1
			// Server
			TestServer server(v4d::io::TCP, &rsa);
			server.Start(44444);

			auto rsaPublicKey = v4d::crypto::RSA::FromPublicKeyPEM(TestClient{v4d::io::TCP}.GetServerPublicKey("127.0.0.1", 44444));

			// Client
			TestClient client(v4d::io::TCP, &rsaPublicKey);
			if (client.Connect("127.0.0.1", 44444, 1)) {
				if (client.id != 16488) {
					LOG_ERROR("Networking Error Test1: Wrong ID")
					return 3;
				}
				SLEEP(100ms) // wait for server to have the time to decrement result
				client.Disconnect();
				server.Stop();
				if (result != 50) {
					LOG_ERROR(result << "  Networking Error Test1: Wrong result after first connect")
					return 4;
				}


				return 0;

				// TestClient client2(client);
				// if (client2.Connect("127.0.0.1", 44444, 2)) {
				// 	// SUCCESS
				// 	SLEEP(100ms) // wait for server to have the time to decrement result
				// 	if (result != 0) {
				// 		LOG_ERROR(result << "  Networking Error Test1: Wrong result after second connect")
				// 	}
				// 	return result;// If fails without error, then final result is not good.
				// } else {
				// 	LOG_ERROR("Networking Error Test1: TOKEN Connection failed")
				// 	return 2;
				// }


			} else {
				LOG_ERROR("Networking Error Test1: AUTH Connection failed")
				return 1;
			}
		}

		return -1;
	}
}
