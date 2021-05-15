#pragma once

#include <v4d.h>

// Dependencies for Modular
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <functional>
#include <filesystem>
#include "utilities/io/FilePath.h"

// // Dependencies for V4D_Mod
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/Window.h"
#include "utilities/graphics/Renderer.h"


class V4DLIB V4D_Mod {
	V4D_MODULE_CLASS_HEADER(V4D_Mod
		,Mod_RunFromConsole
		,Mod_Sort
		
		// Renderer configuration
		,Renderer_ConfigureDeviceExtensions
		,Renderer_ScorePhysicalDeviceSelection
		,Renderer_ConfigureDeviceFeatures
		,Renderer_ConfigureRenderer
		,Renderer_ConfigureLayouts
		,Renderer_ConfigureShaders
		,Renderer_ConfigureRenderPasses
		,Renderer_AllocateResources
		,Renderer_FreeResources
		,Renderer_LoadScene
		,Renderer_UnloadScene
		
		// Called on every rendering frame
		,Renderer_BeforeUpdate
		,Renderer_FrameUpdate
		,Renderer_PushCommands
		,Renderer_VisibilityCommands
		,Renderer_ComputeCommands
		,Renderer_LightingCommands
		,Renderer_PullCommands
		,Renderer_PostCommands
		,Renderer_SwapChainRenderPass
		// Secondary
		,Renderer_SecondaryUpdate
		
	)
	
	V4D_MODULE_FUNC_DECLARE(int, Mod_RunFromConsole, const int argc, const char** argv)
	V4D_MODULE_FUNC_DECLARE(int, Mod_Sort)
	
	// Renderer configuration
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureDeviceExtensions)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ScorePhysicalDeviceSelection)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureDeviceFeatures)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureRenderer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureLayouts)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureShaders)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureRenderPasses)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_AllocateResources)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_FreeResources)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_LoadScene)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_UnloadScene)
	
	// Called on every rendering frame
	V4D_MODULE_FUNC_DECLARE(void, Renderer_BeforeUpdate, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_FrameUpdate, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_PushCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_VisibilityCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ComputeCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_LightingCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_PullCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_PostCommands, VkCommandBuffer cmdBuffer, int currentFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_SwapChainRenderPass, VkCommandBuffer cmdBuffer, int currentFrame, int swapChainImageIndex)
	// Secondary
	V4D_MODULE_FUNC_DECLARE(void, Renderer_SecondaryUpdate)

};
