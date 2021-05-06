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
#include "utilities/graphics/vulkan/Buffer.h"
#include "utilities/graphics/vulkan/DescriptorSet.h"
#include <utilities/graphics/vulkan/RasterShaderPipeline.h>
#include "utilities/io/Logger.h"

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

		// Main Graphics Card
		PhysicalDevice* renderingPhysicalDevice = nullptr;
		Device* renderingDevice = nullptr;
		
		std::vector<DeviceQueueInfo> queuesInfo {
			{
				VK_QUEUE_GRAPHICS_BIT,
				1, // Count
				{1.0f}, // Priorities (one per queue count)
				&surface // Putting a surface here forces the need for a presentation feature on that specific queue family
			},
			// {
			// 	VK_QUEUE_COMPUTE_BIT,
			// },
			// {
			// 	VK_QUEUE_TRANSFER_BIT,
			// },
		};

		// Swap Chains
		SwapChain* swapChain = nullptr;
		uint32_t swapChainImageIndex;
		static constexpr int NB_FRAMES_IN_FLIGHT = V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES;
		uint32_t currentFrame = 0;
		std::vector<VkPresentModeKHR> preferredPresentModes {
			VK_PRESENT_MODE_MAILBOX_KHR,	// TripleBuffering (No Tearing, low latency)
			VK_PRESENT_MODE_IMMEDIATE_KHR,	// VSync OFF (With Tearing, no latency)
			VK_PRESENT_MODE_FIFO_KHR,	// VSync ON (No Tearing, more latency)
		};
		std::vector<VkSurfaceFormatKHR> preferredFormats {
			{VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_HDR10_HLG_EXT},
			// {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		};
		
		// State
		enum class STATE {NONE = 0, INITIALIZED, UNLOADED, LOADED, RUNNING} state = STATE::NONE;
		
		// Synchronized frame execution
		std::mutex frameSyncMutex;
		std::queue<std::function<void()>> syncQueue {};
		std::unique_ptr<std::thread> shaderWatcherThread = nullptr;
		
		// Descriptor sets
		VkDescriptorPool descriptorPool;
		std::map<std::string, DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		
		// Ray-Tracing Shaders
		static std::unordered_map<std::string, uint32_t> sbtOffsets;

	public: // Device Extensions and features
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
		
		void RequiredDeviceExtension(const char* ext) {
			requiredDeviceExtensions.push_back(std::move(ext));
		}

		void OptionalDeviceExtension(const char* ext) {
			optionalDeviceExtensions.push_back(std::move(ext));
		}

		bool IsDeviceExtensionEnabled(const char* ext) {
			return enabledDeviceExtensions.find(ext) != enabledDeviceExtensions.end();
		}

	public: // Pure virtual methods
		virtual void ConfigureDeviceExtensions() = 0;
		virtual void ScorePhysicalDeviceSelection(int& score, PhysicalDevice*) = 0;
		virtual void ConfigureDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* availableDeviceFeatures) = 0;
		virtual void ConfigureRenderer() = 0;
		virtual void ConfigureLayouts() = 0;
		virtual void ConfigureShaders() = 0;
		virtual void ReadShaders() = 0;
		virtual void CreateResources() = 0;
		virtual void DestroyResources() = 0;
		virtual void AllocateBuffers() = 0;
		virtual void FreeBuffers() = 0;
		virtual void CreatePipelines() = 0;
		virtual void DestroyPipelines() = 0;
		virtual void CreateSyncObjects() = 0;
		virtual void DestroySyncObjects() = 0;
		virtual void CreateCommandBuffers() = 0;
		virtual void DestroyCommandBuffers() = 0;
		virtual void Render() = 0;
		
	protected: // FrameBuffered classes
	
		template<class T>
		struct FrameBufferedObject {
			std::array<T, NB_FRAMES_IN_FLIGHT> objArray;
			T& operator[](size_t i) {return objArray[i];}
		};
		
		struct FrameBuffered_Semaphore : FrameBufferedObject<VkSemaphore> {
			void Create(Device* device) {
				VkSemaphoreCreateInfo semaphoreInfo {};
					semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				for (auto&s : objArray) {
					Renderer::CheckVkResult("Create Semaphore", device->CreateSemaphore(&semaphoreInfo, nullptr, &s));
				}
			}
			void Destroy(Device* device) {
				for (auto&s : objArray) {
					device->DestroySemaphore(s, nullptr);
				}
			}
		};
		
		struct FrameBuffered_Fence : FrameBufferedObject<VkFence> {
			void Create(Device* device, bool signaled = true) {
				VkFenceCreateInfo fenceInfo = {};
					fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
					fenceInfo.flags = signaled? VK_FENCE_CREATE_SIGNALED_BIT : 0; // Initialize in the signaled state so that we dont get stuck on the first frame
				for (auto&f : objArray) {
					Renderer::CheckVkResult("Create Fence", device->CreateFence(&fenceInfo, nullptr, &f));
				}
			}
			void Destroy(Device* device) {
				for (auto&f : objArray) device->DestroyFence(f, nullptr);
			}
		};
		
		struct FrameBuffered_CommandBuffer : FrameBufferedObject<VkCommandBuffer> {
			void Allocate(Device* device, VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT, uint queueIndex = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
				VkCommandBufferAllocateInfo allocInfo = {};
					allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
					allocInfo.commandBufferCount = objArray.size();
					allocInfo.level = level;
					allocInfo.commandPool = device->GetQueue(queueFlags, queueIndex).commandPool;
				Renderer::CheckVkResult("Allocate CommandBuffer", device->AllocateCommandBuffers(&allocInfo, objArray.data()));
			}
			void Free(Device* device) {
				device->FreeCommandBuffers(device->GetGraphicsQueue().commandPool, objArray.size(), objArray.data());
			}
		};
		
		struct FrameBuffered_Image : FrameBufferedObject<Image> {
			VkImageUsageFlags usage;
			uint32_t mipLevels;
			uint32_t arrayLayers;
			std::vector<VkFormat> preferredFormats;
			
			FrameBuffered_Image(
				VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				uint32_t mipLevels = 1,
				uint32_t arrayLayers = 1,
				const std::vector<VkFormat>& preferredFormats = {VK_FORMAT_R32G32B32A32_SFLOAT}
			): 	usage(usage),
				mipLevels(mipLevels),
				arrayLayers(arrayLayers),
				preferredFormats(preferredFormats)
			{}
			
			void Create(Device* device, uint32_t width, uint32_t height = 0, const std::vector<VkFormat>& tryFormats = {}, int additionalFormatFeatures = 0) {
				for (auto&i : objArray) {
					i = {usage, mipLevels, arrayLayers, preferredFormats};
					i.Create(device, width, height, tryFormats, additionalFormatFeatures);
				}
			}
			void Destroy(Device* device) {
				for (auto&i : objArray) i.Destroy(device);
			}
			void TransitionLayout(Device* device, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1) {
				for (auto&i : objArray) i.TransitionLayout(device, commandBuffer, oldLayout, newLayout, mipLevels, layerCount);
			}
			operator std::vector<Image*>() {
				std::vector<Image*> vec {};
				for (auto&i : objArray) vec.emplace_back(&i);
				return vec;
			}
		};
		
		
		
	protected: // Virtual INIT Methods

		virtual void CreateDevices();
		virtual void DestroyDevices();

		virtual void CreateCommandPools();
		virtual void DestroyCommandPools();
		
		virtual void CreateDescriptorSets();
		virtual void DestroyDescriptorSets();

		virtual void CreateSwapChain();
		virtual void DestroySwapChain();
		virtual void RecreateSwapChain();
		
		virtual void LoadGraphicsToDevice();
		virtual void UnloadGraphicsFromDevice();
		
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
		
		void RecordAndSubmitCommandBuffer(
			const VkCommandBuffer& commandBuffer,
			const std::function<void(VkCommandBuffer)>& commandsToRecord,
			const std::vector<VkSemaphore>& waitSemaphores = {},
			const std::vector<VkPipelineStageFlags>& waitStages = {},
			const std::vector<VkSemaphore>& signalSemaphores = {},
			const VkFence& triggerFence = VK_NULL_HANDLE
		);
		
		// Present
		bool EndFrame(const std::vector<VkSemaphore>& waitSemaphores = {});
		
	public: // Sync methods
		virtual void UpdateDescriptorSets();
		virtual void UpdateDescriptorSet(DescriptorSet* set, const std::vector<uint32_t>& bindings = {});
		
		/** This is NOT for use every frame, it WILL cause stuttering.
		 *	Typical use case is for hot-reloading shaders or re-create pipelines when things change drastically.
		 *	Anything executed with this function will effectively pause the rendering so that nothing is currently in use on the device.
		 */
		void RunSynchronized(std::function<void()>&& func) {
			std::lock_guard lock(frameSyncMutex);
			syncQueue.emplace(std::move(func));
		}

	public: // Helper methods
	
		VkCommandBuffer BeginSingleTimeCommands(const Queue& queue) {
			return renderingDevice->BeginSingleTimeCommands(queue);
		}

		void EndSingleTimeCommands(const Queue& queue, VkCommandBuffer commandBuffer) {
			renderingDevice->EndSingleTimeCommands(queue, commandBuffer);
		}

		void RunSingleTimeCommands(const Queue& queue, std::function<void(VkCommandBuffer)>&& func) {
			renderingDevice->RunSingleTimeCommands(queue, std::forward<std::function<void(VkCommandBuffer)>>(func));
		}
		

		// void AllocateBufferStaged(const Queue&, Buffer&);
		// void AllocateBuffersStaged(const Queue&, std::vector<Buffer>&);
		// void AllocateBuffersStaged(const Queue&, std::vector<Buffer*>&);

	public: // Init//LoadReset Methods
		
		virtual void InitRenderer();
		virtual void LoadRenderer();
		virtual void UnloadRenderer();
		virtual void ReloadRenderer();
		virtual void ReloadPipelines();
		void WatchModifiedShadersForReload(const std::vector<ShaderPipelineMetaFile>&);
		
		void AssignSurface(const VkSurfaceKHR& surface) {
			this->surface = surface;
		}
		
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
