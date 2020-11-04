#pragma once
#include <v4d.h>

class V4DLIB V4D_Game {
	V4D_MODULE_CLASS_HEADER(V4D_Game
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
		,RendererRayCast
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, LoadScene)
	V4D_MODULE_FUNC_DECLARE(void, UnloadScene)
	V4D_MODULE_FUNC_DECLARE(void, Update, double deltaTime)
	V4D_MODULE_FUNC_DECLARE(void, SlowUpdate, double deltaTime)
	V4D_MODULE_FUNC_DECLARE(void, RendererCreateResources, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC_DECLARE(void, RendererDestroyResources, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC_DECLARE(void, RendererRunUi)
	V4D_MODULE_FUNC_DECLARE(void, RendererRunUiDebug)
	V4D_MODULE_FUNC_DECLARE(void, RendererFrameUpdate)
	V4D_MODULE_FUNC_DECLARE(void, RendererFrameUpdate2)
	V4D_MODULE_FUNC_DECLARE(void, RendererFrameCompute, VkCommandBuffer)
	V4D_MODULE_FUNC_DECLARE(void, RendererRayCast, v4d::graphics::RenderRayCastHit)
};
