#pragma once

// Common Headers (Precompiled)

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

