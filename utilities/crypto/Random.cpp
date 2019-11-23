#include <v4d.h>

using namespace v4d::crypto;

void Random::Generate(std::vector<byte>& bytes) {
	RAND_bytes(bytes.data(), (int)bytes.size());
}
