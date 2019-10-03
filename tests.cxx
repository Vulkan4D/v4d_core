#include "utilities/processing/ThreadPool.cxx"
#include "utilities/networking/networking.cxx"
// #include "utilities/networking/networking_bug.cxx"
#include "helpers/event.cxx"
#include "helpers/Base16.cxx"
#include "helpers/Base64.cxx"
#include "utilities/crypto/AES.cxx"
#include "utilities/crypto/RSA.cxx"
#include "utilities/crypto/SHA.cxx"
#include "utilities/data/DataStream.cxx"
#include "utilities/io/Socket.cxx"

#define RUN_UNIT_TESTS(funcName, ...) { LOG("Running tests for " << #funcName << " ..."); result += funcName(__VA_ARGS__); if (result != 0) { LOG_ERROR("UNIT TESTS FAILED"); return result; } }
#define START_UNIT_TESTS using namespace v4d::tests; int main() { LOG("Started unit tests"); int result = 0; { V4D_PROJECT_INSTANTIATE_CORE_IN_MAIN( v4dCore ) 
#define END_UNIT_TESTS } if (result == 0) LOG_SUCCESS("\nALL TESTS PASSED\n"); return result; }

namespace v4d::tests {
	int V4D_CORE() {
		int result = 0;
		{
			LOGGER_INSTANCE->SetVerbose(true);

			// RUN_UNIT_TESTS( ThreadPool )
			// RUN_UNIT_TESTS( Event )
			// RUN_UNIT_TESTS( Base16 )
			// RUN_UNIT_TESTS( Base64 )
			// RUN_UNIT_TESTS( AES )
			// RUN_UNIT_TESTS( RSA )
			// RUN_UNIT_TESTS( SHA )
			// RUN_UNIT_TESTS( DataStream )
			// RUN_UNIT_TESTS( Socket )
			RUN_UNIT_TESTS( Networking )



		}
		LOG_SUCCESS("V4D_CORE tests PASSED");
		return result;
	}
}
