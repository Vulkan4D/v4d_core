#pragma once

#ifdef _WIN32
	#ifdef _V4D_CORE
		#define IMGUI_API __declspec( dllexport )
	#else
		#define IMGUI_API __declspec( dllimport )
	#endif
#else
	#define IMGUI_API
#endif

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
