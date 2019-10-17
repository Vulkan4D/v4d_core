#include <v4d.h>

namespace v4d::tests {
	int BinaryFileStream() {
		int result = 0;

		if (!v4d::io::FilePath::CreateDirectory("testdir_/someotherdir/stuff")) {
			LOG_ERROR("Failed to create directory")
			return 1;
		}

		if (!v4d::io::FilePath::DeleteDirectory("testdir_", true)) {
			LOG_ERROR("Failed to delete directory")
			return 2;
		}

		{
			v4d::io::BinaryFileStream fileStream(v4d::io::FilePath("testfiles_/test_BinaryFileStream.txt"));

			{// Test 1 (int, bool, float, double... using stream operators)
				result = 100;
				fileStream.Truncate();

				fileStream	<< (int)	54
					<< (bool)	true
					<< (float)	40.5f
					<< (double)	5.5
				;
				fileStream.Flush();

				int a;
				bool b;
				float c;
				double d;

				fileStream	>> a
							>> b
							>> c
							>> d
				;

				if (b) {
					result -= a;
					result -= (int)(c+(float)d);
				}
				if (result != 0) {
					LOG_ERROR(result << " Failed BinaryFileStream test1")
					return result;
				}
			}

			{// Test 2 (long string list)
				result = 100;
				fileStream.Truncate();
				
				std::vector<std::string> strListWrite = {"000", "111", "222", "", "444", LONG_ASS_STRING};
				fileStream << strListWrite;
				fileStream.Flush();

				std::vector<std::string> strListRead;
				fileStream >> strListRead;
				if (strListRead[0] == "000" && strListRead[1] == "111" && strListRead[2] == "222" && strListRead[3] == "" && strListRead[4] == "444" && strListRead[5] == LONG_ASS_STRING) {
					result -= 100;
				}

				if (result != 0) {
					LOG_ERROR(result << " Failed BinaryFileStream test2")
					return result;
				}
			}

			fileStream.Delete();
		}

		if (!v4d::io::FilePath::DeleteDirectory("testfiles_", true)) {
			LOG_ERROR("Failed to delete directory after running tests")
			return 3;
		}

		return result;
	}
}
