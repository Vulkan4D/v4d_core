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
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ScorePhysicalDeviceSelection) {
				mod->Renderer_ScorePhysicalDeviceSelection();
			}
		});
	}
	
	void ConfigureDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* availableDeviceFeatures) override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_ConfigureDeviceFeatures) {
				mod->Renderer_ConfigureDeviceFeatures();
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
	
	void CreateImages() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_CreateImages) {
				mod->Renderer_CreateImages();
			}
		});
	}
	
	void DestroyImages() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_DestroyImages) {
				mod->Renderer_DestroyImages();
			}
		});
	}
	
#pragma endregion
	
#pragma region Scene (final stuff to load when renderer is ready)
	
	void LoadScene() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_LoadScene) {
				mod->Renderer_LoadScene();
			}
		});
	}
	
	void UnloadScene() override {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_UnloadScene) {
				mod->Renderer_UnloadScene();
			}
		});
	}
	
#pragma endregion
	
public:
	
#pragma region Rendering
	
	inline void Render_BeforeUpdate(int currentFrame) {
		V4D_Mod::ForEachSortedModule([currentFrame](auto* mod){
			if (mod->Renderer_BeforeUpdate) {
				mod->Renderer_BeforeUpdate(currentFrame);
			}
		});
	}
	inline void Render_FrameUpdate(int currentFrame) {
		V4D_Mod::ForEachSortedModule([currentFrame](auto* mod){
			if (mod->Renderer_FrameUpdate) {
				mod->Renderer_FrameUpdate(currentFrame);
			}
		});
	}
	inline void Render_PushCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_PushCommands) {
				mod->Renderer_PushCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_VisibilityCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_VisibilityCommands) {
				mod->Renderer_VisibilityCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_ComputeCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_ComputeCommands) {
				mod->Renderer_ComputeCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_LightingCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_LightingCommands) {
				mod->Renderer_LightingCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_PullCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_PullCommands) {
				mod->Renderer_PullCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_PostCommands(VkCommandBuffer cmdBuffer, int currentFrame) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame](auto* mod){
			if (mod->Renderer_PostCommands) {
				mod->Renderer_PostCommands(cmdBuffer, currentFrame);
			}
		});
	}
	inline void Render_SwapChainRenderPass(VkCommandBuffer cmdBuffer, int currentFrame, int swapChainImageIndex) {
		V4D_Mod::ForEachSortedModule([&cmdBuffer, currentFrame, swapChainImageIndex](auto* mod){
			if (mod->Renderer_SwapChainRenderPass) {
				mod->Renderer_SwapChainRenderPass(cmdBuffer, currentFrame, swapChainImageIndex);
			}
		});
	}
	
	inline void Render_SecondaryUpdate() {
		V4D_Mod::ForEachSortedModule([](auto* mod){
			if (mod->Renderer_SecondaryUpdate) {
				mod->Renderer_SecondaryUpdate();
			}
		});
	}

#pragma endregion

};
}
