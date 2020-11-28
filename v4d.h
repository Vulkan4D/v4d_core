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

// Common includes (Mostly STL)
#include "common/includes.hh"

// This includes all helpers (simple header-only source files)
#include "helpers.hh"

// V4D Core class (Compiled into v4d.dll)
#include "Core.h"

// This includes all V4D Core Utilities (Also Compiled into v4d.dll)
#include "utilities.hh"

// Mods
#include "V4D_Mod.h"

// Initial source code for the End-Project
#include "project.hh"
