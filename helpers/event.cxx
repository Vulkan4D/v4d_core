#include <v4d.h>

struct ___TestClassForEvent {int x; int y;};

#ifdef EVENT_DEFINITIONS_VARIADIC
	DEFINE_EVENT(EVT1, ___TestClassForEvent)
	DEFINE_EVENT(EVT2)
	DEFINE_EVENT(EVT3, int, std::string)
	DEFINE_EVENT(EVT4, int)
#else
	DEFINE_EVENT(v4d::tests, EVT1, ___TestClassForEvent)
	DEFINE_EVENT(v4d::tests, EVT2, int)
	DEFINE_EVENT(v4d::tests, EVT3, std::string)
	DEFINE_EVENT(v4d::tests, EVT4, void*)
#endif

namespace v4d::tests {

	int Event() {
		int result = 20;

		#ifdef EVENT_DEFINITIONS_VARIADIC
			v4d::tests::event::EVT1 << [&result](___TestClassForEvent t){ result -= t.x; result -= t.y; };
			v4d::tests::event::EVT1(___TestClassForEvent{4,6});

			v4d::tests::event::EVT2 << [&result](){ result -= 2; };
			v4d::tests::event::EVT2();

			v4d::tests::event::EVT3 << [&result](int a, std::string b){ result -= a; };
			v4d::tests::event::EVT3(3, "test");

			v4d::tests::event::EVT4 << [&result](int a){ result -= a; };
			v4d::tests::event::EVT4(5);
		#else
			v4d::tests::event::EVT1 << [&result](___TestClassForEvent t){ result -= t.x; result -= t.y; };
			v4d::tests::event::EVT1(___TestClassForEvent{4,6});

			v4d::tests::event::EVT2 << [&result](int a){ result -= a; };
			v4d::tests::event::EVT2(5);

			v4d::tests::event::EVT3 << [&result](std::string b){ result -= 3; };
			v4d::tests::event::EVT3("test");

			v4d::tests::event::EVT4 << [&result](void*){ result -= 2; };
			v4d::tests::event::EVT4(nullptr);
		#endif

		return result;
	}
}
