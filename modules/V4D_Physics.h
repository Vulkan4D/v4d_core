#pragma once
#include <v4d.h>

#ifdef _ENABLE_IMGUI
	// #ifdef _DEBUG
		#define __V4D_Physics_IMGUI ,RunImGui, RunImGuiDebug
	// #else
		// #define __V4D_Physics_IMGUI ,RunImGui
	// #endif
#else
	#define __V4D_Physics_IMGUI
#endif

class V4DLIB V4D_Physics {
	V4D_MODULE_CLASS_H(V4D_Physics
		,OrderIndex
		,Init
		// ,LoadScene
		// ,UnloadScene
		// ,CreateResources
		// ,DestroyResources
		// __V4D_Physics_IMGUI
		// ,StepSimulation
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init)
	// V4D_MODULE_FUNC(void, LoadScene)
	// V4D_MODULE_FUNC(void, UnloadScene)
	// V4D_MODULE_FUNC(void, CreateResources, v4d::graphics::vulkan::Device*)
	// V4D_MODULE_FUNC(void, DestroyResources, v4d::graphics::vulkan::Device*)
	// #ifdef _ENABLE_IMGUI
	// 	V4D_MODULE_FUNC(void, RunImGui)
	// 	// #ifdef _DEBUG
	// 		V4D_MODULE_FUNC(void, RunImGuiDebug)
	// 	// #endif
	// #endif
	// V4D_MODULE_FUNC(void, StepSimulation)
};
