#include <v4d.h>

namespace v4d::tests {
	int Socket() {

		{// Test 1 (TCP)
			int result = 100;

			v4d::Socket server(TCP);

			// DOES NOT WORK IN WINDOWS YET...
			server.Bind(44444);

			server.StartListening([](v4d::Socket s){
				auto msg = s.Read<std::string>();
				int a = s.Read<int>();
				s.Write<int>(a * 2);
				s.Write<long>(50);
				s.Write<short>(msg=="Hello Socket!"? 8 : 0);
				s.Flush();
			});

			v4d::Socket client(TCP);
			client.Connect("127.0.0.1", 44444);
			client.Write<std::string>("Hello Socket!");
			client.Write<int>(21);
			client.Flush();

			result -= client.Read<int>();
			result -= client.Read<long>();
			result -= client.Read<short>();

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR("v4d::tests::Socket Error test1 (result " << result << ")")
				return 1;
			}
		}

		// {// Test 2 (UDP)
		// 	int result = 100;

		// 	v4d::Socket server(UDP);
		// 	server.Bind(44444);

		// 	server.StartListeningUDP([&server](){
		// 		auto msg = server.Read<std::string>();
		// 		int a = server.Read<int>();
		// 		server.Write<int>(a * 2);
		// 		server.Write<long>(50);
		// 		server.Write<short>(msg=="Hello Socket!"? 8 : 0);
		// 		server.Flush();
		// 	});

		// 	v4d::Socket client(UDP);
		// 	client.Connect("127.0.0.1", 44444);
		// 	client.Write<std::string>("Hello Socket!");
		// 	client.Write<int>(21);
		// 	client.Flush();

		// 	result -= client.Read<int>();
		// 	result -= client.Read<long>();
		// 	result -= client.Read<short>();

		// 	client.Disconnect();
		// 	server.Disconnect();

		// 	if (result != 0) {
		// 		LOG_ERROR("v4d::tests::Socket Error test2")
		// 		return 1;
		// 	}
		// }

		return 0;
	}
}
