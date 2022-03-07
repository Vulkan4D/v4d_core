#pragma once

//////////////////////////////////////////////////////
#define __V4D_MACRO_STR_(x) #x
#define __V4D_MACRO_STR(x) __V4D_MACRO_STR_(x)
#define V4D_VERSION __V4D_MACRO_STR(V4D_VERSION_MAJOR) "." __V4D_MACRO_STR(V4D_VERSION_MINOR) "." __V4D_MACRO_STR(V4D_VERSION_PATCH)

//////////////////////////////////////////////////////
// Includes (the order is important)

// Config
#include "../v4dconfig.hh"
#if defined(_V4D_APP) || defined(_V4D_GAME) || defined(_V4D_MODULE)
	#include "../../config.hh"
#endif

// Common includes
#include <iostream>
#include <csignal>
#ifdef _V4D_CORE
	// OpenSSL
	#include <openssl/sha.h>
	#include <openssl/pem.h>
	#include <openssl/aes.h>
	#include <openssl/rsa.h>
	#include <openssl/err.h>
	#include <openssl/rand.h>
#endif
#ifdef _WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <windows.h>
	#define MAXFLOAT std::numeric_limits<float>::max()
#else// _LINUX
	#include <dlfcn.h>
	#include <unistd.h>
	#define MAXBYTE 0xFF
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
	#define V4DGAME
#else// Project/Module
	#define V4DLIB DLLIMPORT
	#ifdef _V4D_GAME
		#define V4DGAME DLLEXPORT
	#else// Project/Module
		#define V4DGAME DLLIMPORT
	#endif
#endif

// Helpers (standalone header-only files unaware of V4D)
#include "helpers/types.hpp"
#include "helpers/macros.hpp"
#include "helpers/event.hpp"
#include "helpers/Timer.hpp"
#include "helpers/String.hpp"
#include "helpers/Base16.hpp"
#include "helpers/Base64.hpp"
#include "helpers/BaseN.hpp"
#include "helpers/TextID.hpp"
#include "helpers/EntityComponentSystem.hpp"
#include "helpers/FPSCounter.hpp"
#include "helpers/COMMON_OBJECT.hpp"
#include "helpers/SHARED_OBJECT.hpp"

// Automatically use vulkan validation layers if we are in debug mode on linux, unless we have defined V4D_VULKAN_NO_VALIDATION_LAYERS
#if !defined(V4D_VULKAN_USE_VALIDATION_LAYERS) && (defined(_DEBUG) && defined(_LINUX) && !defined(V4D_VULKAN_NO_VALIDATION_LAYERS))
	#define V4D_VULKAN_USE_VALIDATION_LAYERS
#endif

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

// Some Utilities
#include "utilities/io/Logger.h"

#if defined(_V4D_APP)
	// Initial source code for the Project (App or Game)
	namespace v4d {
		bool CheckCoreVersion() {
			if (V4D_VERSION != v4d::GetCoreBuildVersion()) {
				std::cerr << "V4D Core Library version mismatch (PROJECT:" << V4D_VERSION << ", V4D_CORE:" << v4d::GetCoreBuildVersion() << ")\n";
				return false;
			}
			return true;
		}
		bool Init() {
			ATTACH_SIGNAL_HANDLER(V4D_SIGNAL_HANDLER)
			if (!v4d::CheckCoreVersion()) return false;
			#ifdef V4D_CORE_LOGGER_INSTANCE
				v4d::Core::coreLogger = V4D_CORE_LOGGER_INSTANCE;
			#else
				v4d::Core::coreLogger = V4D_LOGGER_INSTANCE;
			#endif
			return true;
		}
	}
#endif

// Vulkan loader and xvk
#include "utilities/graphics/vulkan/Loader.h"

// ImGui
#ifdef _ENABLE_IMGUI
	#ifndef IMGUI_API
		#define IMGUI_API V4DLIB
		#define IMGUI_IMPL_API V4DLIB
	#endif
	#include "imgui/imgui.h"
	#include "utilities/graphics/imgui_vulkan.h"
	#include "imgui/backends/imgui_impl_glfw.h"
#endif
