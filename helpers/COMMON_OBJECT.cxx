#include <v4d.h>

namespace v4d::tests {
	
	COMMON_OBJECT_CLASS(Something1, int)
	COMMON_OBJECT_CPP(Something1, int)
	std::array<Something1, 3> stuff1;
	
	COMMON_OBJECT_CLASS(Something2, int)
	COMMON_OBJECT_CPP(Something2, int)
	std::vector<Something2> stuff2 {0,1,2};
	
	
	int CommonObjects() {
		
		if (Something1::Count() != 3) {
			LOG_ERROR("1")
			return -1;
		}
		stuff1.fill(1);
		if (Something1::Count() != 3) {
			LOG_ERROR("2")
			return -1;
		}
		
		if (Something2::Count() != 3) {
			LOG_ERROR("3")
			return -1;
		}
		stuff2.push_back(1);
		stuff2.push_back(2);
		stuff2.push_back(3);
		if (Something2::Count() != 6) {
			LOG_ERROR("4")
			return -1;
		}
		stuff2.emplace_back(4);
		if (Something2::Count() != 7) {
			LOG_ERROR("5")
			return -1;
		}
		
		return 0;
	}
}
