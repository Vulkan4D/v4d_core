#include <v4d.h>

namespace v4d::tests {
	int Socket() {

		{// Test 1 (TCP)
			int result = 100;

			v4d::Socket server(TCP);

			server.Bind(44444);

			server.StartListeningThread([](v4d::Socket& s){
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

		{// Test 2 (UDP)
			int result = 100;

			v4d::Socket server(UDP);
			server.Bind(44444);

			server.StartListeningThread([](v4d::Socket& s, int& result){
				std::string msg;
				msg = s.Read<std::string>();
				int a = s.Read<int>();
				int b = s.Read<int>();
				if (msg == "Hello UDP!") result -= a + b + 45;
			}, result);

			SLEEP(100ms)
			
			server.StopListening();

			v4d::Socket client(UDP);
			client.Connect("127.0.0.1", 44444);
			client.Write<std::string>("Hello UDP!");
			client.Write<int>(20);
			client.Write<int>(35);
			client.Flush();

			SLEEP(100ms)
			
			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test2")
				return 2;
			}
		}

		return 0;
	}
}
