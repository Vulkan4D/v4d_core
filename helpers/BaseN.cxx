#include <v4d.h>

namespace v4d::tests {

	int BaseN() {
		int result = 0;

		uint64_t v1 = 2765465;
		std::string v2 = "H2";
		std::string v3 = "G57HKL903FS2";
		std::string v4 = "r94ng6";
		uint64_t v5 = 247533828721UL;
		uint64_t v6 = std::numeric_limits<uint64_t>::max();
		uint64_t v7 = 0;
		std::string v8 = "ZZZZZZZZZZZZ";
		std::string v9 = "fsgs46211e5w3";
		std::string v10 = "ZZZZZZZZZZZZZ"; // must fail
		std::string v11 = "gdfg$"; // must fail
		std::string v12 = "fsgs46211e5w4"; // must fail
		std::string v13 = "gsgs46211e5w3"; // must fail
		if (v4d::BaseN::DecodeLong(v4d::BaseN::EncodeLong(v1)) != v1) return 1;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v2)) != v2) return 2;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v3)) != v3) return 3;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v4, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v4) return 4;
		if (v4d::BaseN::DecodeLong(v4d::BaseN::EncodeLong(v5, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v5) return 5;
		if (v4d::BaseN::DecodeLong(v4d::BaseN::EncodeLong(v6)) != v6) return 6;
		if (v4d::BaseN::DecodeLong(v4d::BaseN::EncodeLong(v7)) != v7) return 7;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v8)) != v8) return 8;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v9, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v9) return 9;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v10)) == v10) return 10;
		if (v4d::BaseN::EncodeLong(v4d::BaseN::DecodeLong(v11)) == v11) return 11;
		if (v4d::BaseN::DecodeLong(v12, v4d::BASE36_LOWER_CHARS) != 0) return 12;
		if (v4d::BaseN::DecodeLong(v13, v4d::BASE36_LOWER_CHARS) != 0) return 13;

		return result;
	}
}
