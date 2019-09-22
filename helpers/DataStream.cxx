#include <v4d.h>

#define LONG_ASS_STRING "3p9nm845y97348 57 4325 4732t5964bn325v435/4g3/5v/ghégéfv étg435 w3 T%$ V%$/$v45/%$C/45/%ff6 & & trFGD $%#$#$ J HG h FFgfsd fsdf sdfbsfw7f48 4 f87 H^%$ % ^$^ %&^&%$#^ *^$^%^&*^&*#@ @*%&( ytrgh df'eèèefààfdêf ebfwu 78ew78wew<fdsf\tdsfd\nfs fdsfdsfsdfsdfsddfsa fdsa fdsaf dsaf dsaf sadf asdf sadfsadf sadf sadfsad f  8439853956387 543 534534tg3 34 4t fsdfsdf"

namespace v4d::tests {
	int DataStream() {
		int result = 100;
		v4d::DataStream bs(1024);

		{// Test 1 (int, bool, float, double... using stream operators)
			bs	<< (int)	54
				<< (bool)	true
				<< (float)	40.5f
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
				result -= (int)(c+(float)d);
			}
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test1")
				return result;
			}
		}

		{// Test 2 (long, short... with a negative value)
			result = 100;
			std::thread tRead([&bs,&result]{
				result -= (int)bs.Read<long>();
				result -= bs.Read<short>();
			});

			bs.Write<long>(-20);
			bs.Write<short>(120);
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test2")
				return result;
			}
		}

		{// Test 3 (multiple flush, each with multiple values of different sizes)
			result = 100;
			std::thread tRead([&bs,&result]{
				result -= bs.Read<int>();
				result -= bs.Read<short>();
				result -= (int)bs.Read<float>();

				int a,b,c;
				double d;
				bs.Read(a, b, c, d);
				result -= (a+b+c+(int)d);
			});

			bs.Write(
				(int)20, 
				(short)30, 
				(float)20
			).Flush();

			bs.Write(5, 10, 5, 10.0).Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test3")
				return result;
			}
		}

		{// Test 4 (std::vector)
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
				LOG_ERROR(result << " Failed DataStream test4")
				return result;
			}
		}

		{// Test 5 (std::array)
			result = 100;
			std::thread tRead([&bs,&result]{
				std::array<int, 4> arr;
				bs >> arr;
				for (uint i = 0; i < 4; i++) {
					result -= arr[i];
				}
			});

			std::array<int, 4> arr = {20, 25, 45, 10};
			bs << arr;
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test5")
				return result;
			}
		}

		{// Test 6 (multidimentional arrays)
			result = 100;
			std::thread tRead([&bs,&result]{
				int arr[4][2];
				bs >> arr;
				for (uint i = 0; i < 4; i++) {
					result -= (arr[i][0] * arr[i][1]);
				}
			});

			int arr[4][2] = {{2, 10}, {5, 5}, {9, 5}, {2, 5}};
			bs << arr;
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test6")
				return result;
			}
		}

		{// Test 7 (simple string)
			result = 100;
			std::thread tRead([&bs,&result]{
				std::string str(bs.Read<std::string>());
				if (str == "Hello !") {
					result -= 100;
				}
			});

			bs << std::string("Hello !");
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test7")
				return result;
			}
		}

		{// Test 8 (long string list)
			result = 100;
			std::thread tRead([&bs,&result]{
				std::vector<std::string> strList;
				bs >> strList;
				if (strList[0] == "000" && strList[1] == "111" && strList[2] == "222" && strList[3] == "" && strList[4] == "444" && strList[5] == LONG_ASS_STRING) {
					result -= 100;
				}
			});

			std::vector<std::string> strList = {"000", "111", "222", "", "444", LONG_ASS_STRING};
			bs << strList;
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test8")
				return result;
			}
		}

		{// Test 9 (Variadic args)
			result = 100;
			std::thread tRead([&bs,&result]{
				int a,b,c;
				double d;
				bs.Read<int, int, int, double>(a,b,c,d);
				result -= a + b + c + (int)d;
			});

			bs.Write<int, int, int, double>(50, 40, 20, -10.0);
			bs.Flush();

			tRead.join();
			if (result != 0) {
				LOG_ERROR(result << " Failed DataStream test9")
				return result;
			}
		}

		return result;
	}
}
