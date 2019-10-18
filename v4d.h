#pragma once

// Vulkan4D Core Header
// This file is the only header to include for anything that is part of V4D


#define V4D_VERSION "0.0.0" 
// Version must be the same for everyting (project, core, all modules...). The version check is automatically done upon loading


//////////////////////////////////////////////////////
// Includes (the order is important)

// Config
#include "v4dconfig.hh"

// This includes all helpers (simple header-only source files)
#include "helpers.hh"

// V4D Core class (Compiled into v4d.dll)
#include "Core.h"

// This includes all V4D Core Utilities (Also Compiled into v4d.dll)
#include "utilities.hh"

// Initial source code for all V4D Modules (Which will all be compiled as individual dlls loaded at runtime, also used for modding/plugins)
#include "modules.hh"

// Initial source code for the End-Project
#include "project.hh"
