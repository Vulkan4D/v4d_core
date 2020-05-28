#pragma once
#include <v4d.h>

class V4DLIB V4D_Renderer {
	V4D_MODULE_CLASS_HEADER(V4D_Renderer
		,OrderIndex
		,RenderOrderIndex
		,ScorePhysicalDeviceSelection
		,Init
		,LoadScene
		,UnloadScene
		,InitDeviceFeatures
		,ConfigureRenderer
		,InitLayouts
		,ConfigureShaders
		,ReadShaders
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
		,DrawOverlayLine
		,DrawOverlayText
		,DrawOverlayCircle
		,DrawOverlaySquare
		,RunUi
		,Update
		,Update2
		,Render
		,Render2
		// Getters
		,GetImage
		,GetPipelineLayout
		,AddShader
		,GetShaderBindingTable
	)
	
	// Executed only once for configuration
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(int, RenderOrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, ScorePhysicalDeviceSelection, int& score, v4d::graphics::vulkan::PhysicalDevice*) // this is executed once for each available GPU
	V4D_MODULE_FUNC_DECLARE(void, Init, v4d::graphics::Renderer*, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, LoadScene)
	V4D_MODULE_FUNC_DECLARE(void, UnloadScene)
	
	// May be Executed many times (ie: when renderer is reloading or screen is resizing)
	V4D_MODULE_FUNC_DECLARE(void, InitDeviceFeatures)
	V4D_MODULE_FUNC_DECLARE(void, ConfigureRenderer)
	V4D_MODULE_FUNC_DECLARE(void, InitLayouts)
	V4D_MODULE_FUNC_DECLARE(void, ConfigureShaders)
	V4D_MODULE_FUNC_DECLARE(void, ReadShaders)
	V4D_MODULE_FUNC_DECLARE(void, CreateSyncObjects)
	V4D_MODULE_FUNC_DECLARE(void, DestroySyncObjects)
	V4D_MODULE_FUNC_DECLARE(void, CreateResources) // Main Renderer module must call V4D_Game::RendererCreateResources
	V4D_MODULE_FUNC_DECLARE(void, DestroyResources) // Main Renderer module must call V4D_Game::RendererDestroyResources
	V4D_MODULE_FUNC_DECLARE(void, CreateCommandBuffers)
	V4D_MODULE_FUNC_DECLARE(void, DestroyCommandBuffers)
	V4D_MODULE_FUNC_DECLARE(void, AllocateBuffers)
	V4D_MODULE_FUNC_DECLARE(void, FreeBuffers)
	V4D_MODULE_FUNC_DECLARE(void, CreatePipelines)
	V4D_MODULE_FUNC_DECLARE(void, DestroyPipelines)
	
	// Executed every frame
	V4D_MODULE_FUNC_DECLARE(void, RunUi) // Main Renderer module must call V4D_Game::RendererRunUi and V4D_Game::RendererRunUiDebug
	V4D_MODULE_FUNC_DECLARE(void, Update) // Main Renderer module must call V4D_Game::RendererFrameUpdate and V4D_Renderer::Render
	V4D_MODULE_FUNC_DECLARE(void, Update2) // Main Renderer module must call V4D_Game::RendererFrameUpdate2 and V4D_Game::RendererFrameCompute and V4D_Renderer::Render2
	V4D_MODULE_FUNC_DECLARE(void, Render, VkCommandBuffer)
	V4D_MODULE_FUNC_DECLARE(void, Render2, VkCommandBuffer)
	
	// Typically only implemented in primary module
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayLine, float x1, float y1, float x2, float y2, glm::vec4 color, float lineWidth)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayText, const char* text, float x, float y, glm::vec4 color, float size)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayCircle, float x, float y, glm::vec4 color, float size, float borderSize)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlaySquare, float x, float y, glm::vec4 color, glm::vec2 size, float borderSize)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::Image*, GetImage, const std::string& name)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::PipelineLayout*, GetPipelineLayout, const std::string& name)
	V4D_MODULE_FUNC_DECLARE(void, AddShader, const std::string& groupName, v4d::graphics::vulkan::RasterShaderPipeline* shader)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::rtx::ShaderBindingTable*, GetShaderBindingTable, const std::string& sbtName)
};
