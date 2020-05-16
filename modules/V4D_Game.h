#pragma once
#include <v4d.h>

class V4DLIB V4D_Game {
	V4D_MODULE_CLASS_H(V4D_Game
		,OrderIndex
		,Init
		,LoadScene
		,UnloadScene
		,Update
		,SlowUpdate
		,RendererCreateResources
		,RendererDestroyResources
		,RendererRunUi
		,RendererRunUiDebug
		,RendererFrameUpdate
		,RendererFrameUpdate2
		,RendererFrameCompute
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Scene*)
	V4D_MODULE_FUNC(void, LoadScene)
	V4D_MODULE_FUNC(void, UnloadScene)
	V4D_MODULE_FUNC(void, Update, double deltaTime)
	V4D_MODULE_FUNC(void, SlowUpdate, double deltaTime)
	V4D_MODULE_FUNC(void, RendererCreateResources, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC(void, RendererDestroyResources, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC(void, RendererRunUi)
	V4D_MODULE_FUNC(void, RendererRunUiDebug)
	V4D_MODULE_FUNC(void, RendererFrameUpdate)
	V4D_MODULE_FUNC(void, RendererFrameUpdate2)
	V4D_MODULE_FUNC(void, RendererFrameCompute, VkCommandBuffer)
};
