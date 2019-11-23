#pragma once
#include <v4d.h>

// V4D Core class and events (Compiled into v4d.dll)


//////////////////////////////////////////////////////////
// V4D events

namespace v4d {
	struct CoreInitEvent {};
	struct CoreDestroyEvent {};
}

DEFINE_CORE_EVENT_HEADER(V4D_CORE_INIT, CoreInitEvent&)
DEFINE_CORE_EVENT_HEADER(V4D_CORE_DESTROY, CoreDestroyEvent&)

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


//////////////////////////////////////////////////////////
// Some utilities prototypes
namespace v4d::io {
	class Logger;
	class ModulesLoader;
	struct ModuleInstance;
}

//////////////////////////////////////////////////////////

namespace v4d {
	V4DLIB const std::string GetCoreBuildVersion() noexcept;

	class V4DLIB Core {
	protected:
		std::string projectName = "V4D Project";
	public:
		bool Init();
		bool Init(std::shared_ptr<v4d::io::Logger> coreLogger);
		static std::shared_ptr<v4d::io::Logger> coreLogger;
		v4d::io::ModulesLoader* modulesLoader;
		void Destroy();
		~Core();
		void SetProjectName(std::string);
		std::string GetProjectName() const;
		v4d::io::ModuleInstance* LoadModule(const std::string& name);
	};
}

//////////////////////////////////////////////////////////
// Smart pointer type for the Core instance
typedef std::shared_ptr<v4d::Core> v4d_core;
typedef std::weak_ptr<v4d::Core> v4d_core_weak;

