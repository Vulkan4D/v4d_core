#include <v4d.h>

namespace v4d::tests {
	int Socket() {

		{// Test 1 (TCP)
			LOG("    Running Test 1")
			int result = 100;

			v4d::io::Socket server(v4d::io::TCP);

			server.Bind(44444);

			server.StartListeningThread(10, [](v4d::io::SharedSocket socket) {
				auto msg = socket->Read<std::string>();
				int a = socket->Read<int>();
				socket->Write<int>(a * 2);
				socket->Write<long>(50);
				socket->Write<short>(msg=="Hello Socket!"? 8 : 0);
				socket->Flush();
			});

			v4d::io::Socket client(v4d::io::TCP);
			client.Connect("127.0.0.1", 44444);
			client.Write<std::string>("Hello Socket!");
			client.Write<int>(21);
			client.Flush();

			result -= client.Read<int>();
			result -= (int)client.Read<long>();
			result -= client.Read<short>();

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test1")
				return 1;
			}
		}

		{// Test 2 (vectors via TCP)
			LOG("    Running Test 2")
			int result = 100;

			v4d::io::Socket server(v4d::io::TCP);

			server.Bind(44444);

			server.StartListeningThread(10, [&result](v4d::io::SharedSocket socket) {
				int in = socket->Read<int>();
				auto in_bytes = socket->Read<std::vector, byte>();
				auto in_ints = socket->Read<std::vector, int>();
				for (byte b : in_bytes) result -= (int)b;
				for (int i : in_ints) result -= i;
				*socket << in;
				socket->Flush();
			});

			v4d::io::Socket client(v4d::io::TCP);
			client.Connect("127.0.0.1", 44444);
			client.Write((int)10);
			std::vector<byte> bytes = {4, 8, 21, 35};
			std::vector<int> ints = {2, 5, 15};
			client.Write(bytes);
			client.Write(ints);
			client.Flush();

			result -= client.Read<int>();

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test2")
				return 1;
			}
		}

		{// Test 3 (UDP)
			LOG("    Running Test 3")
			int result = 100;

			v4d::io::Socket server(v4d::io::UDP);
			server.Bind(44444);

			server.StartListeningThread(10, [&result](v4d::io::SharedSocket socket/*, int& result*/){
				std::string msg = socket->Read<std::string>();
				int a = socket->Read<int>();
				int b = socket->Read<int>();
				if (msg == "Hello UDP!") result -= a + b + 45;
			}/*, std::ref(result)*/);

			v4d::io::Socket client(v4d::io::UDP);
			client.Connect("127.0.0.1", 44444);
			client.Write<std::string>("Hello UDP!");
			client.Write<int>(20);
			client.Write<int>(35);
			client.Flush();

			SLEEP(10ms)

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test3")
				return 2;
			}
		}

		{// Test 4 (DataStream via UDP)
			LOG("    Running Test 4")
			int result = 100;

			v4d::io::Socket server(v4d::io::UDP);
			server.Bind(44444);

			server.StartListeningThread(10, [/*&result*/](v4d::io::SharedSocket socket, int& result){
				auto stream = socket->ReadStream();
				result -= stream.Read<int>();
				result -= stream.Read<short>();
				result -= (int)stream.Read<double>();
			}, std::ref(result));

			v4d::io::Socket client(v4d::io::UDP);
			client.Connect("127.0.0.1", 44444);

			v4d::data::Stream stream(1024);
			stream.Write<int>(20);
			stream.Write<short>(35);
			stream.Write<double>(45.5);

			client.WriteStream(stream);
			client.Flush();

			SLEEP(10ms)

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test4")
				return 3;
			}
		}

		auto rsa = v4d::crypto::RSA(2048, 3);

		{// Test 5 (Encrypted data via TCP)
			LOG("    Running Test 5")
			int result = 100;

			v4d::io::Socket server(v4d::io::TCP);

			server.Bind(44444);

			server.StartListeningThread(10, [&rsa](v4d::io::SharedSocket socket) {
				auto msg = socket->ReadEncrypted<std::string>(&rsa);
				int a = socket->ReadEncrypted<int>(&rsa);
				socket->WriteEncrypted<int>(&rsa, a * 2);
				socket->WriteEncrypted<long>(&rsa, 30);
				socket->WriteEncrypted<short>(&rsa, msg=="Hello Socket!"? 8 : 0);
				socket->Flush();
			});

			v4d::io::Socket client(v4d::io::TCP);
			client.Connect("127.0.0.1", 44444);
			client.WriteEncrypted<std::string>(&rsa, "Hello Socket!");
			client.WriteEncrypted<int>(&rsa, 31);
			client.Flush();

			result -= client.ReadEncrypted<int>(&rsa);
			result -= (int)client.ReadEncrypted<long>(&rsa);
			result -= client.ReadEncrypted<short>(&rsa);

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test5")
				return 1;
			}
		}

		{// Test 6 (Encrypted DataStream via UDP)
			LOG("    Running Test 6")
			int result = 100;

			v4d::io::Socket server(v4d::io::UDP);
			server.Bind(44444);

			server.StartListeningThread(10, [&result, &rsa](v4d::io::SharedSocket socket){
				auto stream = socket->ReadEncryptedStream(&rsa);
				result -= stream.Read<int>();
				result -= stream.Read<short>();
				result -= (int)stream.Read<double>();
			});

			v4d::io::Socket client(v4d::io::UDP);
			client.Connect("127.0.0.1", 44444);

			v4d::data::Stream stream(1024);
			stream.Write<int>(30);
			stream.Write<short>(15);
			stream.Write<double>(55.5);

			client.WriteEncryptedStream(&rsa, stream);
			client.Flush();

			SLEEP(10ms)

			client.Disconnect();
			server.Disconnect();

			if (result != 0) {
				LOG_ERROR(result << " v4d::tests::Socket Error test6")
				return 3;
			}
		}

		return 0;
	}
}
