#pragma once
// #include <v4d.h>
#include <memory>

// V4D Core class and events (Compiled into v4d.dll)


//////////////////////////////////////////////////////////
// V4D events

namespace v4d {
	struct CoreInitEvent {};
	struct CoreDestroyEvent {};
}

// Signals
DEFINE_CORE_EVENT_HEADER(SIGNAL, int)
DEFINE_CORE_EVENT_HEADER(SIGNAL_TERMINATE, int)
DEFINE_CORE_EVENT_HEADER(SIGNAL_INTERUPT, int)
DEFINE_CORE_EVENT_HEADER(SIGNAL_HANGUP, int)
DEFINE_CORE_EVENT_HEADER(SIGNAL_ABORT, int)
DEFINE_CORE_EVENT_HEADER(SIGNAL_QUIT, int)
DEFINE_CORE_EVENT_HEADER(APP_KILLED, int)
DEFINE_CORE_EVENT_HEADER(APP_ERROR, int)

EXTERNC V4DLIB void V4D_SIGNAL_HANDLER(int num);

EXTERNC V4DLIB const char* SIGNALS_STR[32];

//////////////////////////////////////////////////////////
// Some utilities prototypes

namespace v4d::io {
	class Logger;
}

//////////////////////////////////////////////////////////

namespace v4d {
	V4DLIB const std::string GetCoreBuildVersion() noexcept;

	class V4DLIB Core {
	public:
		static std::shared_ptr<v4d::io::Logger> coreLogger;
	};
}
