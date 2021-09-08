#include <v4d.h>
#include "utilities/data/Stream.h"
#include "utilities/data/ReadOnlyStream.h"

namespace v4d::tests {

	int Base16() {
		int result = 100;

		v4d::data::Stream stream(1024);
		stream << 41;
		stream << true;
		stream << std::string("testing Base16...");
		stream << 50;
		stream << 9.0;

		std::string encodedString = v4d::Base16::Encode(stream.GetData());
		v4d::data::ReadOnlyStream decodedStream(v4d::Base16::Decode(encodedString));

		int a = 0;
		bool b = false;
		std::string c = "";
		int d = 0;
		double e = 0.0;

		decodedStream >> a >> b >> c >> d >> e;

		if (b && c == "testing Base16...") {
			result -= a;
			result -= d;
			result -= (int)e;
		}

		return result;
	}
}
