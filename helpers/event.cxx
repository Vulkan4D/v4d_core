#include "v4d.h"

struct ___TestClassForEvent {
	int test() {
		return 2;
	}
};

DEFINE_EVENT_TYPE(TEST1)
DEFINE_EVENT_TYPE(TEST2, int)
DEFINE_EVENT_TYPE(TEST3, std::string, float)
DEFINE_EVENT_TYPE(TEST4, ___TestClassForEvent)

namespace v4d::tests {

	int Event() {
		int result = 10;
		v4d::event::TEST1 << [&result](){result--;};
		v4d::event::TEST2 << [&result](int a){result -= a;};
		v4d::event::TEST3 << [&result](std::string b, int a){result -= a;};
		v4d::event::TEST4 << [&result](___TestClassForEvent a){result -= a.test();};

		v4d::event::TEST1();
		v4d::event::TEST2(4);
		v4d::event::TEST3("test", 3);
		v4d::event::TEST4(___TestClassForEvent());

		return result;
	}
}
