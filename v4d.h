#pragma once

// Vulkan4D Core Header
// This file is the only header to include for anything that is part of V4D


#define V4D_VERSION_MAJOR 0
#define V4D_VERSION_MINOR 0
#define V4D_VERSION_PATCH 0


//////////////////////////////////////////////////////
#define __V4D_MACRO_STR_(x) #x
#define __V4D_MACRO_STR(x) __V4D_MACRO_STR_(x)
#define V4D_VERSION __V4D_MACRO_STR(V4D_VERSION_MAJOR) "." __V4D_MACRO_STR(V4D_VERSION_MINOR) "." __V4D_MACRO_STR(V4D_VERSION_PATCH)

//////////////////////////////////////////////////////
// Includes (the order is important)

// Config
#include "../v4dconfig.hh"
#ifdef _V4D_PROJECT
	#include "../../config.hh"
#endif
#ifdef _V4D_MODULE
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
	#include <winsock2.h>
	#include <windows.h>
	#define MAXFLOAT std::numeric_limits<float>::max()
#else// _LINUX
	#include <dlfcn.h>
	#include <unistd.h>
	#define MAXBYTE 0xFF
#endif

// Helpers (simple header-only files unaware of V4D)
#include "helpers/types.hpp"
#include "helpers/macros.hpp"
#include "helpers/event.hpp"
#include "helpers/Timer.hpp"
#include "helpers/String.hpp"
#include "helpers/Base16.hpp"
#include "helpers/Base64.hpp"
#include "helpers/BaseN.hpp"
#include "helpers/TextID.hpp"
#include "helpers/modular.hpp"
#include "helpers/EntityComponentSystem.hpp"

// V4D Core class (Compiled into v4d.dll)
#include "Core.h"

#if defined(_V4D_PROJECT)
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
