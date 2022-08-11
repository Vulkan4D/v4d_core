#pragma once

#include <v4d.h>
#include "helpers/modular.hpp"

// Dependencies for Modular
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <functional>
#include <filesystem>
#include "utilities/io/FilePath.h"

// Dependencies for V4D_Mod
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/Window.h"
#include "utilities/graphics/Renderer.h"
#include "utilities/graphics/vulkan/PhysicalDevice.h"

class V4DLIB V4D_Mod {
	V4D_MODULE_CLASS_HEADER(V4D_Mod
		,Mod_RunFromConsole
		,Mod_Sort
		
		// Init Renderer
		,Renderer_ConfigureRenderer
		,Renderer_ConfigureLayouts
		,Renderer_ConfigureShaders
		
		// Load renderer
		,Renderer_ConfigureDeviceExtensions
		,Renderer_ScorePhysicalDeviceSelection
		,Renderer_ConfigureDeviceFeatures
		,Renderer_ConfigureScene
		,Renderer_CreateResources
		,Renderer_DestroyResources
		,Renderer_LoadBuffers
		,Renderer_UnloadBuffers
		,Renderer_ConfigureImages
		,Renderer_ConfigureRenderPasses
		,Renderer_Start
		,Renderer_End
		
		// Frame (Called from GameRenderer for now, will move into the engine at some point in the future)
		,Renderer_BeforeUpdate
		,Renderer_DrawUi
		,Renderer_BeforeFrame
		,Renderer_FrameUpdate
		,Renderer_PushCommands
		,Renderer_BuildCommands
		,Renderer_RenderCommands
		,Renderer_ResolveCommands
		,Renderer_BlitCommands
		,Renderer_PostCommands
		,Renderer_AfterFrame
		,Renderer_AfterUpdate
	)
	
	V4D_MODULE_FUNC_DECLARE(int, Mod_RunFromConsole, const int argc, const char** argv)
	V4D_MODULE_FUNC_DECLARE(int, Mod_Sort)
	
	// Renderer configuration
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureDeviceExtensions)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ScorePhysicalDeviceSelection, int& score, v4d::graphics::vulkan::PhysicalDevice* device)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureDeviceFeatures, v4d::graphics::vulkan::PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const v4d::graphics::vulkan::PhysicalDevice::DeviceFeatures* availableDeviceFeatures)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureRenderer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureLayouts)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureShaders)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureRenderPasses)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureScene)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_CreateResources)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_DestroyResources)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_LoadBuffers)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_UnloadBuffers)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ConfigureImages, uint32_t swapChainWidth, uint32_t swapChainHeight)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_Start)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_End)

	// Called on every rendering frame
	V4D_MODULE_FUNC_DECLARE(void, Renderer_BeforeUpdate)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_DrawUi)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_BeforeFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_FrameUpdate)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_PushCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_BuildCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_RenderCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_ResolveCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_BlitCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_PostCommands, VkCommandBuffer cmdBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_AfterFrame)
	V4D_MODULE_FUNC_DECLARE(void, Renderer_AfterUpdate)
	
};
