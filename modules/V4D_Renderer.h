#pragma once
#include <v4d.h>

#ifdef _ENABLE_IMGUI
	#define __V4D_Renderer_IMGUI ,RunImGui
#else
	#define __V4D_Renderer_IMGUI
#endif

class V4DLIB V4D_Renderer {
	V4D_MODULE_CLASS_H(V4D_Renderer
		,OrderIndex
		,ScorePhysicalDeviceSelection
		,Init
		,InitDeviceFeatures
		,ConfigureRenderer
		,InitLayouts
		,ConfigureShaders
		,ReadShaders
		,LoadScene
		,UnloadScene
		,CreateSyncObjects
		,DestroySyncObjects
		,CreateResources
		,DestroyResources
		,CreateCommandBuffers
		,DestroyCommandBuffers
		,AllocateBuffers
		,FreeBuffers
		,CreatePipelines
		,DestroyPipelines
		,FrameUpdate
		__V4D_Renderer_IMGUI
		,Render
		,Render2
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, ScorePhysicalDeviceSelection, int& score, v4d::graphics::vulkan::PhysicalDevice*)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Renderer*)
	V4D_MODULE_FUNC(void, InitDeviceFeatures)
	V4D_MODULE_FUNC(void, ConfigureRenderer)
	V4D_MODULE_FUNC(void, InitLayouts)
	V4D_MODULE_FUNC(void, ConfigureShaders)
	V4D_MODULE_FUNC(void, ReadShaders)
	V4D_MODULE_FUNC(void, LoadScene)
	V4D_MODULE_FUNC(void, UnloadScene)
	V4D_MODULE_FUNC(void, CreateSyncObjects)
	V4D_MODULE_FUNC(void, DestroySyncObjects)
	V4D_MODULE_FUNC(void, CreateResources)
	V4D_MODULE_FUNC(void, DestroyResources)
	V4D_MODULE_FUNC(void, CreateCommandBuffers)
	V4D_MODULE_FUNC(void, DestroyCommandBuffers)
	V4D_MODULE_FUNC(void, AllocateBuffers)
	V4D_MODULE_FUNC(void, FreeBuffers)
	V4D_MODULE_FUNC(void, CreatePipelines)
	V4D_MODULE_FUNC(void, DestroyPipelines)
	V4D_MODULE_FUNC(void, FrameUpdate, v4d::graphics::Scene&)
	#ifdef _ENABLE_IMGUI
		V4D_MODULE_FUNC(void, RunImGui)
	#endif
	V4D_MODULE_FUNC(void, Render)
	V4D_MODULE_FUNC(void, Render2)
};
