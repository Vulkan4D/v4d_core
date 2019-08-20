// Vulkan4D Core Header

#pragma once

#define V4D_VERSION "0.0.1"

#ifdef _V4D_CORE
	#ifdef _WINDOWS
		#include "common_core.windows.hh"
	#else
		#include "common_core.linux.hh"
	#endif
#else
	#ifdef _WINDOWS
		#include "common.windows.hh"
	#else
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
	#define V4DSYSTEM_CLASS(className) EXTERNC class DLLEXPORT className
#else// Project/Core
	#define V4DSYSTEM
	#define V4DSYSTEM_CLASS(className)
#endif


//////////////////////////////////////////////////////////
// HELPERS

#include "helpers/types.hpp"
#include "helpers/macros.hpp"
#include "helpers/Timer.hpp"
#include "helpers/Logger.hpp"
#include "helpers/error.hpp"
#include "helpers/SystemsLoader.hpp"
#include "helpers/event.hpp"


//////////////////////////////////////////////////////////
// MODULES

#include "modules/processing/ThreadPool.h"


//////////////////////////////////////////////////////////
// V4D global events

DEFINE_CORE_EVENT_HEADER(V4D_CORE_INIT, int)
DEFINE_CORE_EVENT_HEADER(V4D_CORE_DESTROY, int)


//////////////////////////////////////////////////////////
// V4D global functions

namespace v4d {
	V4DLIB std::string GET_V4D_CORE_BUILD_VERSION();
	V4DLIB void Init();
	V4DLIB void Destroy();
}


//////////////////////////////////////////////////////////
// SYSTEMS

#ifdef _V4D_SYSTEM
	V4DSYSTEM std::string GET_V4D_SYSTEM_BUILD_VERSION() {
		return V4D_VERSION;
	}
#endif

