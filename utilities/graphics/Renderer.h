#pragma once

#include <v4d.h>
#include <vector>
#include <thread>
#include <unordered_map>
#include <string>
#include <queue>

#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Instance.h"
#include "utilities/graphics/vulkan/PhysicalDevice.h"
#include "utilities/graphics/vulkan/Queue.hpp"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/SwapChain.h"
#include "utilities/graphics/vulkan/PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/ShaderPipelineObject.h"
#include "utilities/graphics/vulkan/RenderPassObject.h"
#include <utilities/graphics/vulkan/RasterShaderPipelineObject.h>
#include "utilities/graphics/vulkan/CommonObjects.h"
#include "utilities/graphics/vulkan/BufferObject.h"
#include "utilities/graphics/FrameBufferedObject.hpp"

// Useful macro to be used in ConfigureDeviceFeatures()
#define V4D_ENABLE_DEVICE_FEATURE(F) \
	([deviceFeaturesToEnable, availableDeviceFeatures] () -> bool {\
		if (availableDeviceFeatures-> F) {deviceFeaturesToEnable-> F = VK_TRUE; return true;}\
		else {LOG_WARN("Device feature not available: " << #F); return false;}\
	})();
#define V4D_ENABLE_DEVICE_FEATURES(...) FOR_EACH(V4D_ENABLE_DEVICE_FEATURE, __VA_ARGS__)

namespace v4d::graphics {
	using namespace v4d::graphics::vulkan;

	#pragma region Pack Helpers
	
	V4DLIB glm::f32 PackColorAsFloat(glm::vec4 color);
	V4DLIB glm::u32 PackColorAsUint(glm::vec4 color);
	V4DLIB glm::vec4 UnpackColorFromFloat(glm::f32 color);
	V4DLIB glm::vec4 UnpackColorFromUint(glm::u32 color);
	V4DLIB glm::f32 PackUVasFloat(glm::vec2 uv);
	V4DLIB glm::u32 PackUVasUint(glm::vec2 uv);
	V4DLIB glm::vec2 UnpackUVfromFloat(glm::f32 uv);
	V4DLIB glm::vec2 UnpackUVfromUint(glm::u32 uv);

	#pragma endregion
	
	// struct RayCast {
	// 	uint64_t moduleVen = 0;
	// 	uint64_t moduleId = 0;
	// 	uint64_t objId = 0;
	// 	uint64_t raycastCustomData = 0;
	// 	glm::vec3 localSpaceHitPosition;
	// 	glm::f32 distance;
	// 	glm::vec4 localSpaceHitSurfaceNormal; // w component is unused
		
	// 	bool operator==(const RayCast& other) const {
	// 		return moduleVen == other.moduleVen && moduleId == other.moduleId && objId == other.objId;
	// 	}
	// 	bool operator!=(const RayCast& other) const {
	// 		return !(*this == other);
	// 	}
	// 	operator bool () const {
	// 		return moduleVen && moduleId;
	// 	}
	// };
	
	class V4DLIB Renderer : public Instance {
	protected:
	
		// Main Render Surface
		VkSurfaceKHR surface;
		
		// Synchronized frame execution
		std::mutex frameSyncMutex;
		std::recursive_mutex frameSyncMutex2;
		std::queue<std::function<void()>> syncQueue {};
		std::vector<std::unique_ptr<std::thread>> shaderWatcherThreads {};
		
		std::vector<const char*> requiredDeviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			#ifdef V4D_VULKAN_USE_VMA
				VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
				VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
			#endif
		};
		std::vector<const char*> optionalDeviceExtensions {};
		std::vector<const char*> deviceExtensions {};
		std::unordered_map<std::string, bool> enabledDeviceExtensions {};
		
	public:
	
		// Main Graphics Card
		PhysicalDevice* renderingPhysicalDevice = nullptr;
		Device* renderingDevice = nullptr;
		
		// Swap Chains
		SwapChain* swapChain = nullptr;
		uint32_t swapChainImageIndex;
		static constexpr int NB_FRAMES_IN_FLIGHT = V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES;
		uint32_t currentFrame = 0;
		std::vector<VkPresentModeKHR> preferredPresentModes {
			VK_PRESENT_MODE_MAILBOX_KHR,	// TripleBuffering (No Tearing, low latency)
			VK_PRESENT_MODE_IMMEDIATE_KHR,	// VSync OFF (With Tearing, no latency)
			VK_PRESENT_MODE_FIFO_KHR,		// VSync ON (No Tearing, more latency)
			VK_PRESENT_MODE_FIFO_RELAXED_KHR,
		};
		std::vector<VkSurfaceFormatKHR> preferredFormats {
			{VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_HDR10_HLG_EXT},
			// {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		};
		
		std::vector<DeviceQueueInfo> queuesInfo {
			{
				VK_QUEUE_GRAPHICS_BIT,
				2, // Count
				{1.0f, 0.1f}, // Priorities (one per queue count)
				&surface // Putting a surface here forces the need for a presentation feature on that specific queue family
			},
			{
				VK_QUEUE_COMPUTE_BIT,
			},
			{
				VK_QUEUE_TRANSFER_BIT,
			},
		};

		// State
		enum class STATE {NONE = 0, INITIALIZED, UNLOADED, LOADED, RUNNING} state = STATE::NONE;
		
		// Descriptor sets
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		
		// Ray-Tracing Shaders
		static std::unordered_map<std::string, uint32_t> sbtOffsets;

	public: // Device Extensions and features
	
		void RequiredDeviceExtension(const char* ext) {
			requiredDeviceExtensions.push_back(std::move(ext));
		}

		void OptionalDeviceExtension(const char* ext) {
			optionalDeviceExtensions.push_back(std::move(ext));
		}

		bool IsDeviceExtensionEnabled(const char* ext) {
			return enabledDeviceExtensions.find(ext) != enabledDeviceExtensions.end();
		}

	protected: // Pure virtual methods
	
		virtual void ConfigureDeviceExtensions() = 0;
		virtual void ScorePhysicalDeviceSelection(int& score, PhysicalDevice*) = 0;
		virtual void ConfigureDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* availableDeviceFeatures) = 0;
		virtual void ConfigureRenderer() = 0;
		virtual void ConfigureLayouts() = 0;
		virtual void ConfigureShaders() = 0;
		virtual void ConfigureRenderPasses() = 0;
		virtual void ConfigureImages(uint32_t swapChainWidth, uint32_t swapChainHeight) = 0;
		virtual void LoadBuffers() = 0;
		virtual void UnloadBuffers() = 0;
		virtual void LoadScene() = 0;
		virtual void UnloadScene() = 0;
		virtual void Render() = 0;
		
	protected: // Virtual INIT Methods

		virtual void CreateDevices();
		virtual void DestroyDevices();

		virtual void CreateDescriptorSets();
		virtual void DestroyDescriptorSets();

		virtual void CreateSwapChain();
		virtual void DestroySwapChain();
		virtual void RecreateSwapChain();
		
		virtual void LoadGraphicsToDevice();
		virtual void UnloadGraphicsFromDevice();
		
		// Command Pools
		void CreateCommandPools() {
			for (auto&[k, qs] : renderingDevice->GetQueues()) if (k) {
				for (auto& q : qs) {
					if (!q.commandPool) {
						renderingDevice->CreateCommandPool(q.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &q.commandPool);
					}
				}
			}
		}
		void DestroyCommandPools() {
			for (auto&[k, qs] : renderingDevice->GetQueues()) if (k) {
				for (auto& q : qs) {
					if (q.commandPool) {
						renderingDevice->DestroyCommandPool(q.commandPool);
					}
				}
			}
		}

		// Command buffers
		virtual void CreateCommandBuffers() {
			CommandBufferObject::ForEach([this](CommandBufferObject*o){o->Allocate(renderingDevice);});
		}
		virtual void DestroyCommandBuffers() {
			CommandBufferObject::ForEach([this](CommandBufferObject*o){o->Free();});
		}
		// Synchronization objects
		virtual void CreateSyncObjects() {
			SemaphoreObject::ForEach([this](SemaphoreObject*o){o->Create(renderingDevice);});
			FenceObject::ForEach([this](FenceObject*o){o->Create(renderingDevice);});
		}
		virtual void DestroySyncObjects() {
			SemaphoreObject::ForEach([this](SemaphoreObject*o){o->Destroy();});
			FenceObject::ForEach([this](FenceObject*o){o->Destroy();});
		}
		// Pipeline layouts
		virtual void CreatePipelineLayouts() {
			PipelineLayoutObject::ForEach([this](PipelineLayoutObject*o){o->Create(renderingDevice);});
		}
		virtual void DestroyPipelineLayouts() {
			PipelineLayoutObject::ForEach([this](PipelineLayoutObject*o){o->Destroy();});
		}
		// Shader Pipelines
		virtual void CreateShaderPipelines() {
			ShaderPipelineObject::ForEach([this](ShaderPipelineObject*o){o->Create(renderingDevice);});
		}
		virtual void DestroyShaderPipelines() {
			ShaderPipelineObject::ForEach([this](ShaderPipelineObject*o){o->Destroy();});
		}
		virtual void ReadShaders() {
			ShaderPipelineObject::ForEach([](ShaderPipelineObject*o){o->ReadShaders();});
		}
		// Render Passes
		virtual void CreateRenderPasses() {
			RenderPassObject::ForEach([this](RenderPassObject*o){o->Create(renderingDevice);});
		}
		virtual void DestroyRenderPasses() {
			RenderPassObject::ForEach([](RenderPassObject*o){o->Destroy();});
		}
		// Buffers
		virtual void CreateBuffers() {
			BufferObject::ForEach([this](BufferObject*o){o->Allocate(renderingDevice);});
		}
		virtual void DestroyBuffers() {
			BufferObject::ForEach([](BufferObject*o){o->Free();});
		}
		// Textures
		virtual void LoadTextures() {
			//TODO
		}
		virtual void UnloadTextures() {
			//TODO
		}
		// Images
		virtual void CreateImages() {
			ImageObject::ForEach([this](ImageObject*img){
				if (img->squareSize != 0) {
					img->width = img->squareSize;
					img->height = img->squareSize;
				}
				if (img->width == 0) img->width = img->scale!=1.0? uint32_t(std::round(img->scale * float(swapChain->extent.width))) : swapChain->extent.width;
				if (img->height == 0) img->height = img->scale!=1.0? uint32_t(std::round(img->scale * float(swapChain->extent.height))) : swapChain->extent.height;
				img->Create(renderingDevice);
			});
			RunSingleTimeCommands(renderingDevice->GetGraphicsQueue(), [this](VkCommandBuffer cmdBuffer){
				ImageObject::ForEach([this,&cmdBuffer](ImageObject*img){
					img->TransitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				});
			});
		}
		virtual void DestroyImages() {
			ImageObject::ForEach([this](ImageObject*img){
				img->Destroy();
			});
		}

		
		[[nodiscard("Must not continue current frame if BeginFrame() returns false")]]
		virtual bool BeginFrame(VkSemaphore triggerSemaphore, VkFence triggerFence = VK_NULL_HANDLE);

		inline virtual bool BeginFrame(const VkFence& triggerFence) {
			return BeginFrame(VK_NULL_HANDLE, triggerFence);
		}

		void WaitForFence(VkFence& fence) {
			if (fence == VK_NULL_HANDLE) return;
			CheckVkResult("Wait for Fence", renderingDevice->WaitForFences(1, &fence, VK_TRUE, /*timeout*/1000UL * 1000 * 1000));
		}
		
		void ResetFence(const VkFence& fence) {
			if (fence == VK_NULL_HANDLE) return;
			CheckVkResult("Reset Fence", renderingDevice->ResetFences(1, &fence));
		}
		
		// Present
		bool EndFrame(const std::vector<VkSemaphore>& waitSemaphores = {});
		
		virtual void UpdateDescriptorSets();
		// virtual void UpdateDescriptorSet(DescriptorSet* set, const std::vector<uint32_t>& bindings = {});
		
	public: // Sync methods
		
		/** This is NOT for use every frame, it WILL cause stuttering.
		 *	Typical use case is for hot-reloading shaders or re-create pipelines when things change drastically.
		 *	Anything executed with this function will effectively pause the rendering so that nothing is currently in use on the device.
		 */
		void RunSynchronized(std::function<void()>&& func) {
			std::lock_guard lock(frameSyncMutex);
			syncQueue.emplace(std::move(func));
		}

	public: // Commands
	
		void RecordAndSubmitCommandBuffer(
			const VkCommandBuffer& commandBuffer,
			const VkQueue& queue,
			const std::function<void(VkCommandBuffer)>& commandsToRecord,
			const std::vector<VkSemaphore>& waitSemaphores = {},
			const std::vector<VkPipelineStageFlags>& waitStages = {},
			const std::vector<VkSemaphore>& signalSemaphores = {},
			const VkFence& triggerFence = VK_NULL_HANDLE
		);
		
		void RecordAndSubmitCommandBuffer(
			CommandBufferObject& commandBuffer,
			std::function<void(VkCommandBuffer)>&& commandsToRecord,
			const std::vector<VkSemaphore>& waitSemaphores = {},
			const std::vector<VkPipelineStageFlags>& waitStages = {},
			const std::vector<VkSemaphore>& signalSemaphores = {},
			const VkFence& triggerFence = VK_NULL_HANDLE
		){
			RecordAndSubmitCommandBuffer(
				commandBuffer,
				commandBuffer.GetQueue().handle,
				std::forward<std::function<void(VkCommandBuffer)>>(commandsToRecord),
				waitSemaphores,
				waitStages,
				signalSemaphores,
				triggerFence
			);
		}
		
		VkCommandBuffer BeginSingleTimeCommands(const Queue& queue) {
			return renderingDevice->BeginSingleTimeCommands(queue);
		}

		void EndSingleTimeCommands(const Queue& queue, VkCommandBuffer commandBuffer) {
			renderingDevice->EndSingleTimeCommands(queue, commandBuffer);
		}

		void RunSingleTimeCommands(const Queue& queue, std::function<void(VkCommandBuffer)>&& func) {
			renderingDevice->RunSingleTimeCommands(queue, std::forward<std::function<void(VkCommandBuffer)>>(func));
		}
		
	public: // Init//LoadReset Methods
		
		virtual void InitRenderer();
		virtual void LoadRenderer();
		virtual void UnloadRenderer();
		virtual void ReloadRenderer();
		virtual void ReloadShaderPipelines();
		void WatchModifiedShadersForReload(const std::vector<ShaderPipelineMetaFile>&);
		
		void AssignSurface(const VkSurfaceKHR& surface) {
			this->surface = surface;
		}
		
		#ifdef _ENABLE_IMGUI
			ImGuiIO* imGuiIO = nullptr;
			inline bool IsImGuiReady() const {
				return imGuiIO->Fonts->IsBuilt();
			};
			GLFWwindow* imGuiGlfwWindow = nullptr;
			void InitImGui(GLFWwindow*);
			void LoadImGui(VkRenderPass);
			void UnloadImGui();
			void BeginFrameImGui();
			void EndFrameImGui();
			void DrawImGui(VkCommandBuffer commandBuffer);
		#endif
		
	public: // Constructor & Destructor
		Renderer(Loader* loader, const char* applicationName, uint applicationVersion);
		virtual ~Renderer() override;
		
	};
}

DEFINE_EVENT(v4d::graphics::renderer, PipelinesCreate, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, PipelinesDestroy, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Load, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Unload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Reload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Resize, Renderer*)
