#pragma once

// Vulkan4D Core Header
// This file is the only header to include for anything that is part of V4D


#define V4D_VERSION "0.0.0" 


//////////////////////////////////////////////////////
// Includes (the order is important)

// Config
#include "../v4dconfig.hh"
#ifdef _V4D_PROJECT
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

// Initial source code for the End-Project
#include "project.hh"
