#include <v4d.h>
#include "utilities/data/Stream.h"
#include "utilities/data/ReadOnlyStream.h"

namespace v4d::tests {

	int Base64() {
		int result = 100;

		v4d::data::Stream stream(1024);
		stream << 40;
		stream << true;
		stream << std::string("testing Base64...");
		stream << 50;
		stream << 10.0;

		std::string encodedString = v4d::Base64::Encode(stream.GetData());
		v4d::data::ReadOnlyStream decodedStream(v4d::Base64::Decode(encodedString));

		int a = 0;
		bool b = false;
		std::string c = "";
		int d = 0;
		double e = 0.0;

		decodedStream >> a >> b >> c >> d >> e;

		if (b && c == "testing Base64...") {
			result -= a;
			result -= d;
			result -= (int)e;
		}

		return result;
	}
}
