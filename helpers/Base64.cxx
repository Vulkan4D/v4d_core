#include <v4d.h>

namespace v4d::tests {

	int Base64() {
		int result = 100;

		v4d::Stream stream(1024);
		stream << 40;
		stream << true;
		stream << std::string("testing...");
		stream << 50;
		stream << 10.0;

		std::string encodedString = v4d::Base64::Encode(stream._GetWriteBuffer_().data(), stream._GetWriteBuffer_().size());
		v4d::ReadOnlyStream decodedStream(v4d::Base64::Decode(encodedString));

		int a;
		bool b;
		std::string c;
		int d;
		double e;

		decodedStream >> a >> b >> c >> d >> e;

		result -= a;
		result -= d;
		result -= (int)e;

		return result;
	}
}
