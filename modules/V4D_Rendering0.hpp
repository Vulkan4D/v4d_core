#pragma once
#include <v4d.h>

#ifdef _ENABLE_IMGUI
	#define __V4D_Rendering0_IMGUI ,RunImGui
#else
	#define __V4D_Rendering0_IMGUI
#endif

V4D_MODULE_CLASS_BEGIN(V4D_Rendering0
		,Init
		,ScorePhysicalDeviceSelection
		,OrderIndex
		__V4D_Rendering0_IMGUI
		,LoadScene
		,FrameUpdate
		,FreeBuffers
		,AllocateBuffers
	)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Renderer*)
	V4D_MODULE_FUNC(void, ScorePhysicalDeviceSelection, int& score, v4d::graphics::vulkan::PhysicalDevice*)
	V4D_MODULE_FUNC(int, OrderIndex)
	#ifdef _ENABLE_IMGUI
		V4D_MODULE_FUNC(void, RunImGui)
	#endif
	V4D_MODULE_FUNC(void, LoadScene, v4d::graphics::Scene&)
	V4D_MODULE_FUNC(void, FrameUpdate, v4d::graphics::Scene&)
	V4D_MODULE_FUNC(void, FreeBuffers)
	V4D_MODULE_FUNC(void, AllocateBuffers)
V4D_MODULE_CLASS_END()
