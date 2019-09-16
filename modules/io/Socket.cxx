#include <v4d.h>

namespace v4d::tests {
	int Socket() {

		{// Test 1 (TCP)
			int result = 100;

			v4d::io::Socket server(v4d::io::TCP);

			server.Bind(44444);

			server.StartListeningThread([](v4d::io::Socket& s){
				auto msg = s.Read<std::string>();
				int a = s.Read<int>();
				s.Write<int>(a * 2);
				s.Write<long>(50);
				s.Write<short>(msg=="Hello Socket!"? 8 : 0);
				s.Flush();
			});

			v4d::io::Socket client(v4d::io::TCP);
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

			v4d::io::Socket server(v4d::io::UDP);
			server.Bind(44444);

			server.StartListeningThread([](v4d::io::Socket& s, int& result){
				std::string msg = s.Read<std::string>();
				int a = s.Read<int>();
				int b = s.Read<int>();
				if (msg == "Hello UDP!") result -= a + b + 45;
			}, result);

			SLEEP(100ms)
			
			server.StopListening();

			v4d::io::Socket client(v4d::io::UDP);
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

		{//TODO test 3 (DataStream via UDP)
			int result = 100;

			v4d::io::Socket server(v4d::io::UDP);
			server.Bind(44444);

			server.StartListeningThread([](v4d::io::Socket& s, int& result){
				auto stream = s.ReadStream();
				result -= stream.Read<int>();
				result -= stream.Read<short>();
				result -= stream.Read<double>();
				result -= 9.5;
			}, result);

			SLEEP(100ms)
			
			server.StopListening();

			v4d::io::Socket client(v4d::io::UDP);
			client.Connect("127.0.0.1", 44444);

			v4d::Stream stream(1024);
			stream.Write<int>(20);
			stream.Write<short>(35);
			stream.Write<double>(35.5);

			client.WriteStream(stream);
			client.Flush();

			SLEEP(100ms)
			
			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test3")
				return 3;
			}
		}

		return 0;
	}
}
