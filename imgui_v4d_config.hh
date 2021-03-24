#pragma once

#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS

#ifndef IMGUI_API
	#ifdef _WINDOWS
		#ifdef _V4D_CORE
			#define IMGUI_API __declspec( dllexport )
		#else
			#define IMGUI_API __declspec( dllimport )
		#endif
	#endif
#endif
