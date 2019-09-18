// Vulkan4D Core Header

#pragma once

#define V4D_VERSION "0.0.1"

#ifdef _V4D_CORE
	#ifdef _WINDOWS
		#include "common_core.windows.hh"
	#else// _LINUX
		#include "common_core.linux.hh"
	#endif
#else
	#ifdef _WINDOWS
		#include "common.windows.hh"
	#else// _LINUX
		#include "common.linux.hh"
	#endif
#endif


//////////////////////////////////////////////////////////
// Library import/export

#define EXTERNC extern "C"
#define EXTERNCPP extern "C++"
#if defined _WINDOWS
	#define DLLEXPORT __declspec(dllexport)
	#define DLLIMPORT __declspec(dllimport)
#else
	#define DLLEXPORT
	#define DLLIMPORT
#endif
#ifdef _V4D_CORE
	#define V4DLIB DLLEXPORT
#else// Project/System
	#define V4DLIB DLLIMPORT
#endif
#ifdef _V4D_SYSTEM
	#define V4DSYSTEM EXTERNC DLLEXPORT
	// https://www.tldp.org/HOWTO/pdf/C++-dlopen.pdf
	// #define V4DSYSTEM_CLASS(className) EXTERNCPP class DLLEXPORT className
#else// Project/Core
	#define V4DSYSTEM
	// #define V4DSYSTEM_CLASS(className)
#endif


//////////////////////////////////////////////////////////
// HELPERS

#include "helpers/types.hpp"
#include "helpers/macros.hpp"
#include "helpers/event.hpp"
#include "helpers/Timer.hpp"
#include "helpers/Stream.hpp"
#include "helpers/DataStream.hpp"
#include "helpers/ReadOnlyStream.hpp"


//////////////////////////////////////////////////////////
// V4D global events

namespace v4d {
	struct CoreInitEvent {};
	struct CoreDestroyEvent {};
}

DEFINE_CORE_EVENT_HEADER(V4D_CORE_INIT, CoreInitEvent&)
DEFINE_CORE_EVENT_HEADER(V4D_CORE_DESTROY, CoreDestroyEvent&)


//////////////////////////////////////////////////////////
// V4D Core and global functions

// Modules prototypes
namespace v4d::io {
	class Logger;
	class SystemsLoader;
	class SystemInstance;
}

namespace v4d {
	V4DLIB const std::string GetCoreBuildVersion() noexcept;
	class V4DLIB Core {
	protected:
		std::string projectName = "V4D Project";
	public:
		bool Init();
		bool Init(std::shared_ptr<v4d::io::Logger> coreLogger);
		static std::shared_ptr<v4d::io::Logger> coreLogger;
		v4d::io::SystemsLoader* systemsLoader;
		void Destroy();
		~Core();
		void SetProjectName(std::string);
		std::string GetProjectName() const;
		v4d::io::SystemInstance* LoadSystem(const std::string& name);
	};
}
typedef std::shared_ptr<v4d::Core> v4d_core;


//////////////////////////////////////////////////////////
// MODULES

#include "modules/io/Logger.h"
#include "modules/processing/ThreadPool.h"
#include "modules/io/SystemsLoader.h"
#include "modules/io/Socket.h"
#include "modules/networking/OutgoingConnection.h"
#include "modules/networking/IncomingConnection.h"
#include "modules/networking/ListeningServer.h"



//////////////////////////////////////////////////////////
// SYSTEMS

#ifdef _V4D_SYSTEM
	v4d::Core* v4dCore;

	V4DSYSTEM std::string __GetCoreBuildVersion() {
		return V4D_VERSION;
	}
	V4DSYSTEM std::string __GetSystemName() {
		return THIS_SYSTEM_NAME;
	}
	V4DSYSTEM int __GetSystemRevision() {
		return THIS_SYSTEM_REVISION;
	}
	V4DSYSTEM std::string __GetSystemDescription() {
		return THIS_SYSTEM_DESCRIPTION;
	}
	V4DSYSTEM void __InitSystem(v4d::Core* instance) {
		v4dCore = instance;
	}
#endif


//////////////////////////////////////////////////////////
// PROJECT

#ifdef _V4D_PROJECT

	namespace v4d {
		bool CheckCoreVersion() {
			if (V4D_VERSION != v4d::GetCoreBuildVersion()) {
				LOG_ERROR("V4D Core Library version mismatch (PROJECT:" << V4D_VERSION << ", V4D_CORE:" << v4d::GetCoreBuildVersion() << ")")
				return false;
			}
			return true;
		}
	}

	#define V4D_PROJECT_INSTANTIATE_CORE_IN_MAIN(v4dCore, ...) \
		if (!v4d::CheckCoreVersion()) return -1; \
		v4d_core v4dCore = std::make_shared<v4d::Core>(); \
		v4dCore->systemsLoader = new v4d::io::SystemsLoader(v4dCore); \
		if (!v4dCore->Init(__VA_ARGS__)) { \
			return -1; \
		}
	
#endif
