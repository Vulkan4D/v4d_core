#include "v4d.h"

struct ___TestClassForEvent {int x; int y;};

DEFINE_EVENT(EVT1, ___TestClassForEvent)
// DEFINE_EVENT(EVT2)
// DEFINE_EVENT(EVT3, int, std::string)
DEFINE_EVENT(EVT4, int)

namespace v4d::tests {

	int Event() {
		int result = 15;

		v4d::event::EVT1 << [&result](___TestClassForEvent t){ result -= t.x; result -= t.y; };
		v4d::event::EVT1({4,6});

		// v4d::event::EVT2 << [&result](){ result -= 2; };
		// v4d::event::EVT2();

		// v4d::event::EVT3 << [&result](int a, std::string b){ result -= a; };
		// v4d::event::EVT3(3);

		v4d::event::EVT4 << [&result](int a){ result -= a; };
		v4d::event::EVT4(5);

		return result;
	}
}
