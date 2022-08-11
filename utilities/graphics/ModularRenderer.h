#pragma once

#include <v4d.h>
#include <V4D_Mod.h>
#include <utilities/graphics/Renderer.h>

namespace v4d::graphics {
using namespace v4d::graphics::vulkan;

class V4DLIB ModularRenderer : public Renderer {
public:

using Renderer::Renderer;
~ModularRenderer();

#pragma region Vulkan Device setup
	
	void ConfigureDeviceExtensions() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureDeviceExtensions) {
				mod->Renderer_ConfigureDeviceExtensions();
			}
		});
	}
	
	void ScorePhysicalDeviceSelection(int& score, PhysicalDevice* device) override {
		V4D_Mod::ForEachSortedModule([&](auto* mod){
			if (mod->Renderer_ScorePhysicalDeviceSelection) {
				mod->Renderer_ScorePhysicalDeviceSelection(score, device);
			}
		});
	}
	
	void ConfigureDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* availableDeviceFeatures) override {
		V4D_Mod::ForEachSortedModule([&](auto* mod){
			if (mod->Renderer_ConfigureDeviceFeatures) {
				mod->Renderer_ConfigureDeviceFeatures(deviceFeaturesToEnable, availableDeviceFeatures);
			}
		});
	}
	
#pragma endregion

#pragma region Renderer configuration
	
	void ConfigureRenderer() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureRenderer) {
				mod->Renderer_ConfigureRenderer();
			}
		});
	}
	
	void ConfigureLayouts() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureLayouts) {
				mod->Renderer_ConfigureLayouts();
			}
		});
	}

#pragma endregion

#pragma region Shaders
	
	void ConfigureShaders() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureShaders) {
				mod->Renderer_ConfigureShaders();
			}
		});
	}
	
#pragma endregion

#pragma region Render passes

	void ConfigureRenderPasses() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureRenderPasses) {
				mod->Renderer_ConfigureRenderPasses();
			}
		});
	}
	
#pragma endregion

#pragma region Scene

	void ConfigureScene() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureScene) {
				mod->Renderer_ConfigureScene();
			}
		});
	}
	
#pragma endregion

#pragma region Resources

	void CreateResources() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_CreateResources) {
				mod->Renderer_CreateResources();
			}
		});
	}
	
	void DestroyResources() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_DestroyResources) {
				mod->Renderer_DestroyResources();
			}
		});
	}
	
#pragma endregion

#pragma region Buffers

	void LoadBuffers() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_LoadBuffers) {
				mod->Renderer_LoadBuffers();
			}
		});
	}
	
	void UnloadBuffers() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_UnloadBuffers) {
				mod->Renderer_UnloadBuffers();
			}
		});
	}
	
#pragma endregion

#pragma region Images
	
	void ConfigureImages(uint32_t swapChainWidth, uint32_t swapChainHeight) override {
		V4D_Mod::ForEachSortedModule([swapChainWidth, swapChainHeight](auto* mod){
			if (mod->Renderer_ConfigureImages) {
				mod->Renderer_ConfigureImages(swapChainWidth, swapChainHeight);
			}
		});
	}
	
#pragma endregion
	
#pragma region Start/End (final stuff to load when renderer is ready)
	
	void Start() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_Start) {
				mod->Renderer_Start();
			}
		});
	}
	
	void End() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_End) {
				mod->Renderer_End();
			}
		});
	}
	
#pragma endregion

	void Render() override {
		//TODO: implement a basic renderer that calls all the other module functions, with proper synchronization in between
		/*
		Renderer_BeforeUpdate()
		Renderer_DrawUi()
		Renderer_BeforeFrame()
		Renderer_FrameUpdate()
		Renderer_PushCommands()
		Renderer_BuildCommands()
		Renderer_RenderCommands()
		Renderer_ResolveCommands()
		Renderer_BlitCommands()
		Renderer_PostCommands()
		Renderer_AfterFrame()
		Renderer_AfterUpdate()
		*/
	}
	
};
}
