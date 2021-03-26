#include <v4d.h>

namespace v4d::tests {
	
	int RunBaseNTests() {
		uint64_t v1 = 2765465;
		std::string v2 = "H2";
		std::string v3 = "G57HKL903FS2";
		std::string v4 = "r94ng6";
		uint64_t v5 = 247533828721UL;
		uint64_t v6 = std::numeric_limits<uint64_t>::max();
		uint64_t v7 = 0;
		std::string v8 = "ZZZZZZZZZZZZ";
		std::string v9 = "fsgs46211e5w";
		std::string v10 = "ZZZZZZZZZZZZZ"; // must fail (too long)
		std::string v11 = "gdfg$"; // must fail (wrong character)
		std::string v12 = "fsgs46211e5w4"; // must fail (too long)
		std::string v13 = "gsGs46211e5d"; // must fail (wrong character)
		std::string v14 = "a2kG6j9S0_.d2"; // must fail (too long)
		std::string v15 = "a2_G6-9Z2b"; // must fail (wrong character)
		std::string v16 = "a2_G6.9Z2b";
		std::string v17 = "ZZZZZZZZZZZZZ";
		std::string v18 = "AAAAAAAAAAAAA";
		std::string v19 = "0000";
		std::string v20 = "..........";
		std::string v21 = "G7.82 B_82-";
		std::string v22 = "           ";
		
		if (v4d::BaseN::EncodeStringToUInt64(v4d::BaseN::DecodeStringFromUInt64(v1)) != v1) return 1;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v2)) != v2) return 2;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v3)) != v3) return 3;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v4, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v4) return 4;
		if (v4d::BaseN::EncodeStringToUInt64(v4d::BaseN::DecodeStringFromUInt64(v5, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v5) return 5;
		if (v4d::BaseN::EncodeStringToUInt64(v4d::BaseN::DecodeStringFromUInt64(v6)) != v6) return 6;
		if (v4d::BaseN::EncodeStringToUInt64(v4d::BaseN::DecodeStringFromUInt64(v7)) != v7) return 7;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v8)) != v8) return 8;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v9, v4d::BASE36_LOWER_CHARS), v4d::BASE36_LOWER_CHARS) != v9) return 9;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v10)) == v10) return 10;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v11)) == v11) return 11;
		if (v4d::BaseN::EncodeStringToUInt64(v12, v4d::BASE36_LOWER_CHARS) != 0) return 12;
		if (v4d::BaseN::EncodeStringToUInt64(v13, v4d::BASE36_LOWER_CHARS) != 0) return 13;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v14, v4d::BASE64_WORD_DOT_CHARS), v4d::BASE64_WORD_DOT_CHARS) == v14) return 14;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v15, v4d::BASE64_WORD_DOT_CHARS), v4d::BASE64_WORD_DOT_CHARS) == v15) return 15;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v16, v4d::BASE64_WORD_DOT_CHARS), v4d::BASE64_WORD_DOT_CHARS) != v16) return 16;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v17, v4d::BASE26_UPPER_CHARS), v4d::BASE26_UPPER_CHARS) != v17) return 17;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v18, v4d::BASE26_UPPER_CHARS), v4d::BASE26_UPPER_CHARS) != v18) return 18;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v19, v4d::BASE64_WORD_DOT_CHARS), v4d::BASE64_WORD_DOT_CHARS) != v19) return 19;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v20, v4d::BASE64_WORD_DOT_CHARS), v4d::BASE64_WORD_DOT_CHARS) != v20) return 20;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v21, v4d::BASE40_UPPER_CHARS), v4d::BASE40_UPPER_CHARS) != v21) return 21;
		if (v4d::BaseN::DecodeStringFromUInt64(v4d::BaseN::EncodeStringToUInt64(v22, v4d::BASE40_UPPER_CHARS), v4d::BASE40_UPPER_CHARS) != v22) return 22;
		
		return 0;
	}

	int BaseN() {
		int result = RunBaseNTests();
		return result;
	}
}
