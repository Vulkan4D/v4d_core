#pragma once
#include <v4d.h>

namespace v4d::graphics {
	using namespace v4d::graphics::vulkan;

	#pragma region Pack Helpers
	
	/////////////////
	// NOT WORKING
			// static NormalBuffer_T PackNormal(glm::vec3 normal) {
			// 	// // vec2
			// 	// float f = glm::sqrt(8.0f * normal.z + 8.0f);
			// 	// return glm::vec2(normal) / f + 0.5f;
				
			// 	// vec4
			// 	return glm::vec4(normal, 0);
			// }

			// static glm::vec3 UnpackNormal(NormalBuffer_T norm) {
			// 	// glm::vec2 fenc = norm * 4.0f - 2.0f;
			// 	// float f = glm::dot(fenc, fenc);
			// 	// float g = glm::sqrt(1.0f - f / 4.0f);
			// 	// return glm::vec3(fenc * g, 1.0f - f / 2.0f);
			// 	return norm;
			// }
	/////////////////

	V4DLIB glm::f32 PackColorAsFloat(glm::vec4 color);
	V4DLIB glm::u32 PackColorAsUint(glm::vec4 color);
	V4DLIB glm::vec4 UnpackColorFromFloat(glm::f32 color);
	V4DLIB glm::vec4 UnpackColorFromUint(glm::u32 color);
	V4DLIB glm::f32 PackUVasFloat(glm::vec2 uv);
	V4DLIB glm::u32 PackUVasUint(glm::vec2 uv);
	V4DLIB glm::vec2 UnpackUVfromFloat(glm::f32 uv);
	V4DLIB glm::vec2 UnpackUVfromUint(glm::u32 uv);

	#pragma endregion
	
	struct RenderRayCastHit {
		glm::vec3 position; // relative to center of hit object
		float distance;
		uint32_t objId;
		uint32_t flags;
		uint8_t customType;
		uint32_t customData0;
		uint32_t customData1;
		uint32_t customData2;
	};
	
	class V4DLIB Renderer : public Instance {
	public: // class members

		// Main Render Surface
		VkSurfaceKHR surface;

		// Main Graphics Card
		PhysicalDevice* renderingPhysicalDevice = nullptr;
		Device* renderingDevice = nullptr;
		
		std::vector<DeviceQueueInfo> queuesInfo {
			{
				"graphics",
				VK_QUEUE_GRAPHICS_BIT,
				1, // Count
				{1.0f}, // Priorities (one per queue count)
				&surface // Putting a surface here forces the need for a presentation feature on that specific queue family
			},
			{
				"transfer",
				VK_QUEUE_TRANSFER_BIT,
			},
			{
				"compute",
				VK_QUEUE_COMPUTE_BIT,
			},
		};

		// Swap Chains
		SwapChain* swapChain = nullptr;
		size_t currentFrameInFlight = 0;
		size_t nextFrameInFlight = 0;
		static constexpr int NB_FRAMES_IN_FLIGHT = 2;
		
		// Descriptor sets
		VkDescriptorPool descriptorPool;
		std::map<std::string, DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		
	public: // States
		bool mustReload = false;
		bool graphicsLoadedToDevice = false;
		std::thread::id renderThreadId = std::this_thread::get_id();
		std::recursive_mutex renderMutex1, renderMutex2;
		
	public: // Preferences

		std::vector<VkPresentModeKHR> preferredPresentModes {
			// VK_PRESENT_MODE_MAILBOX_KHR,	// TripleBuffering (No Tearing, low latency)
			// VK_PRESENT_MODE_FIFO_KHR,	// VSync ON (No Tearing, more latency)
			VK_PRESENT_MODE_IMMEDIATE_KHR,	// VSync OFF (With Tearing, no latency)
		};
		std::vector<VkSurfaceFormatKHR> preferredFormats {
			{VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_HDR10_HLG_EXT},
			{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		};

	public: // Device Extensions and features
		std::vector<const char*> requiredDeviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			#ifdef V4D_VULKAN_USE_VMA
				VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
			#endif
		};
		std::vector<const char*> optionalDeviceExtensions {};
		std::vector<const char*> deviceExtensions {};
		std::unordered_map<std::string, bool> enabledDeviceExtensions {};
		
		template<typename T>
		static void FilterSupportedDeviceFeatures(T* enabledFeatures, T supportedFeatures, size_t offset = 0) {
			const size_t featuresArraySize = (sizeof(T)-offset) / sizeof(VkBool32);
			VkBool32 supportedFeaturesData[featuresArraySize];
			VkBool32 enabledFeaturesData[featuresArraySize];
			memcpy(supportedFeaturesData, ((byte*)&supportedFeatures)+offset, sizeof(supportedFeaturesData));
			memcpy(enabledFeaturesData, ((byte*)enabledFeatures)+offset, sizeof(enabledFeaturesData));
			for (size_t i = 0; i < featuresArraySize; ++i) {
				if (enabledFeaturesData[i] && !supportedFeaturesData[i]) {
					enabledFeaturesData[i] = VK_FALSE;
				}
			}
			memcpy(((byte*)enabledFeatures+offset), enabledFeaturesData, sizeof(enabledFeaturesData));
		}
		
		// These objects will be modified to keep only the supported & enabled values
		VkPhysicalDeviceFeatures deviceFeatures {};
		VkPhysicalDeviceVulkan12Features vulkan12DeviceFeatures {};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures {};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures {};
		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures {};
		
		VkPhysicalDeviceVulkan12Features* EnableVulkan12DeviceFeatures();
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR* EnableRayTracingPipelineFeatures();
		VkPhysicalDeviceAccelerationStructureFeaturesKHR* EnableAccelerationStructureFeatures();
		VkPhysicalDeviceRayQueryFeaturesKHR* EnableRayQueryFeatures();
		
		void RequiredDeviceExtension(const char* ext);
		void OptionalDeviceExtension(const char* ext);
		bool IsDeviceExtensionEnabled(const char* ext);

	public: // Virtual methods
		// Init
		virtual void InitDeviceFeatures();
		virtual void ConfigureRenderer();
		virtual void InitLayouts();
		virtual void ConfigureShaders();
		
		// Scene
		virtual void ReadShaders();
		
		// Resources
		virtual void CreateResources();
		virtual void DestroyResources();
		virtual void AllocateBuffers();
		virtual void FreeBuffers();
		
		// Pipelines
		virtual void CreatePipelines();
		virtual void DestroyPipelines();
		
	protected: // Virtual INIT Methods

		virtual void CreateDevices();
		virtual void DestroyDevices();

		virtual void CreateSyncObjects();
		virtual void DestroySyncObjects();
		
		virtual void CreateCommandPools();
		virtual void DestroyCommandPools();
		
		virtual void CreateDescriptorSets();
		virtual void DestroyDescriptorSets();

		virtual bool CreateSwapChain();
		virtual void DestroySwapChain();

		virtual void CreateCommandBuffers();
		virtual void DestroyCommandBuffers();

	public: // Sync methods
		virtual void UpdateDescriptorSets();
		virtual void UpdateDescriptorSet(DescriptorSet* set, const std::vector<uint32_t>& bindings = {});

	public: // Helper methods
	
		VkCommandBuffer BeginSingleTimeCommands(const Queue&);
		void EndSingleTimeCommands(const Queue&, VkCommandBuffer);
		void RunSingleTimeCommands(const Queue&, std::function<void(VkCommandBuffer)>&&);

		void AllocateBufferStaged(const Queue&, Buffer&);
		void AllocateBuffersStaged(const Queue&, std::vector<Buffer>&);
		void AllocateBuffersStaged(const Queue&, std::vector<Buffer*>&);
		
		// void AllocateBufferStaged(Buffer&);
		// void AllocateBuffersStaged(std::vector<Buffer>&);
		// void AllocateBuffersStaged(std::vector<Buffer*>&);

		// void TransitionImageLayout(Image image, VkImageLayout oldLayout, VkImageLayout newLayout);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, Image image, VkImageLayout oldLayout, VkImageLayout newLayout);
		// void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, VkImageAspectFlags aspectMask = 0);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, VkImageAspectFlags aspectMask = 0);

		// void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		// void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		// void GenerateMipmaps(Texture2D* texture);

		// void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels);

	public: // Init//LoadReset Methods
		virtual void RecreateSwapChains();
		
		virtual void InitRenderer();
		virtual void LoadRenderer();
		virtual void UnloadRenderer();
		virtual void ReloadRenderer();
		
		virtual void LoadGraphicsToDevice();
		virtual void UnloadGraphicsFromDevice();
		
	public: // Constructor & Destructor
		Renderer(Loader* loader, const char* applicationName, uint applicationVersion);
		virtual ~Renderer() override;
		
	public: // Public Update Methods
		virtual void Update();
	};
}

DEFINE_EVENT(v4d::graphics::renderer, PipelinesCreate, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, PipelinesDestroy, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Load, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Unload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Reload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Resize, Renderer*)
