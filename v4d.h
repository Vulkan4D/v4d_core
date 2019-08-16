// Vulkan4D Core Header

#pragma once

#define V4D_VERSION "0.0.1"

#ifdef _WINDOWS
	#include "common_windows.hh"
#else
	#include "common_linux.hh"
#endif


//////////////////////////////////////////////////////////
// Library import/export

#define EXTERN extern "C++"
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
	#define V4DSYSTEM EXTERN DLLEXPORT
	#define V4DSYSTEM_CLASS(className) EXTERN class DLLEXPORT className
#else// Project/Core
	#define V4DSYSTEM EXTERN DLLIMPORT
	#define V4DSYSTEM_CLASS(className) EXTERN class DLLIMPORT className
#endif


//////////////////////////////////////////////////////////
// HELPERS

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
// V4D global functions

namespace v4d {
	V4DLIB std::string GET_V4D_CORE_BUILD_VERSION();
	V4DLIB void Init();
	V4DLIB void Destroy();
}


//////////////////////////////////////////////////////////
// V4D global events

// DEFINE_EVENT_TYPE(V4D_CORE_INIT)
// DEFINE_EVENT_TYPE(V4D_CORE_DESTROY)


//////////////////////////////////////////////////////////
// SYSTEMS

#ifdef _V4D_SYSTEM
	V4DSYSTEM std::string GET_V4D_SYSTEM_BUILD_VERSION() {
		return V4D_VERSION;
	}
#endif

