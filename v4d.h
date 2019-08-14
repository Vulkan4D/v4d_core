// Vulkan4D Core Header
#pragma once

#define V4D_VERSION "0.0.1"

#ifdef _WINDOWS
    #include "common_windows.hh"
#else
    #include "common_linux.hh"
#endif


//////////////////////////////////////////////////////////
// HELPERS
#include "helpers/macros.hpp"
#include "helpers/Timer.hpp"
#include "helpers/Logger.hpp"
#include "helpers/error.hpp"
#include "helpers/SystemsLoader.hpp"


//////////////////////////////////////////////////////////
// SYSTEMS LOADER
#ifdef _V4D_SYSTEM
    V4DSYSTEM std::string GET_V4D_SYSTEM_BUILD_VERSION() {
        return V4D_VERSION;
    }
#endif
