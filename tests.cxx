#include "modules/processing/ThreadPool.cxx"
#include "helpers/event.cxx"
#include "helpers/ByteStream.cxx"

#define RUN_UNIT_TESTS(funcName) { LOG("Running tests for " << #funcName << " ..."); result += funcName(); if (result != 0) { LOG_ERROR("UNIT TESTS FAILED"); return result; } }
#define START_UNIT_TESTS using namespace v4d::tests; int main() { LOG("Started unit tests"); int result = 0; {
#define END_UNIT_TESTS } if (result == 0) LOG_SUCCESS("ALL TESTS PASSED"); return result; }

namespace v4d::tests {
	int V4D_CORE() {
		int result = 0;
		{
			
			RUN_UNIT_TESTS( ThreadPool )
			RUN_UNIT_TESTS( Event )
			RUN_UNIT_TESTS( ByteStream )




		}
		LOG_SUCCESS("V4D_CORE tests PASSED");
		return result;
	}
}
