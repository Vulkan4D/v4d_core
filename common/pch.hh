// Common Precompiled Header (Must be included in all .cpp files just before including v4d.h)

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

