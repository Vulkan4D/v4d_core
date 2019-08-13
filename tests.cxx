#pragma once

#include "modules/processing/ThreadPool.cxx"

namespace v4d::tests {
	int RunAllTests() {
		int result = 0;
		{
			
			result += ThreadPool();


		}
		if (result == 0) {
			LOG_SUCCESS("ALL TESTS PASSED");
		} else {
			LOG_ERROR("TESTS FAILED");
		}
		return result;
	}
}
