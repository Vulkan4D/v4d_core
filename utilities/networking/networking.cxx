#include <v4d.h>

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
			authStream->Read<std::string>(); // bob
			return 1; // ID
		} else {
			// Anonymous
			return 0;
		}
	}

	void RunClient(v4d::io::SharedSocket, std::shared_ptr<v4d::networking::IncomingClient>, byte /*clientType*/) override {
		//...
	}

};

class TestClient : public v4d::networking::OutgoingConnection{
public:
	using OutgoingConnection::OutgoingConnection;

	std::string GetAppName() const override {
		return "V4D";
	}
	std::string GetVersion() const override {
		return "0.0.0";
	}

	void Authenticate(v4d::data::Stream* authStream) override {
		authStream->Write<std::string>("bob");
	}
	
	void Run(v4d::io::Socket& /*socket*/) override {
		//...
	}

};

namespace v4d::tests {
	int Networking() {
		v4d::crypto::RSA rsa(2048, 3);

		{// Test1
			// Server
			TestServer server(v4d::io::TCP, &rsa);
			server.Start(44444);
			// Client
			TestClient client(v4d::io::TCP, &rsa);
			if (client.Connect("127.0.0.1", 44444, 1)) {
				return 0;
			} else {
				LOG_ERROR("Networking Error Test1: Connection failed")
				return 1;
			}
		}

		return 0;
	}
}
