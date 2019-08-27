#include <v4d.h>

namespace v4d::tests {
	int ByteStream() {
		int result = 100;
		v4d::ByteStream bs(1024);

		{// Test 1
			bs  << (int)	54
				<< (bool)	true
				<< (float)	40.5
				<< (double)	5.5
			;
			bs.Flush();

			int a;
			bool b;
			float c;
			double d;

			bs	>> a
				>> b
				>> c
				>> d
			;

			if (b) {
				result -= a;
				result -= (c+d);
			}
			if (result != 0) {
				LOG_ERROR("Failed ByteStream test1")
				return result;
			}
		}

		{// Test 2
			result = 100;
			std::thread tRead([&bs,&result]{
				result -= bs.Read<long>();
				result -= bs.Read<short>();
			});

			bs.Write<long>(-20);
			bs.Write<short>(120);
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR("Failed ByteStream test2")
				return result;
			}
		}

		{// Test 3
			result = 100;
			std::thread tRead([&bs,&result]{
				result -= bs.Read<int>();
				result -= bs.Read<short>();
				result -= bs.Read<float>();

				int a,b,c;
				double d;
				bs.Read(a, b, c, d);
				result -= (a+b+c+d);
			});

			bs.Write(
				(int)20, 
				(short)30, 
				(float)20
			).Flush();

			bs.Write(5, 10, 5, 10.0).Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR("Failed ByteStream test3")
				return result;
			}
		}

		{// Test 4
			result = 100;
			std::thread tRead([&bs,&result]{
				for (const int& item : bs.Read<std::vector, int>()) {
					result -= item;
				}
			});

			std::vector<int> list = {15, 25, 50, 10};
			bs << list;
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed ByteStream test4")
				return result;
			}
		}

		return result;
	}
}
