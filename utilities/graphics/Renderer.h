#pragma once
#include <v4d.h>

namespace v4d::graphics {
	using namespace v4d::graphics::vulkan;

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
		const int NB_FRAMES_IN_FLIGHT = 2;
		
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
			{VK_FORMAT_R32G32B32A32_SFLOAT, VK_COLOR_SPACE_HDR10_HLG_EXT},
			{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		};

	public: // Device Extensions and features
		std::vector<const char*> requiredDeviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
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
		VkPhysicalDeviceRayTracingFeaturesKHR rayTracingFeatures {};
		
		VkPhysicalDeviceVulkan12Features* EnableVulkan12DeviceFeatures();
		VkPhysicalDeviceRayTracingFeaturesKHR* EnableRayTracingFeatures();
		
		void RequiredDeviceExtension(const char* ext);
		void OptionalDeviceExtension(const char* ext);
		bool IsDeviceExtensionEnabled(const char* ext);

	public: // Virtual methods
		// Init
		virtual void Init();
		virtual void InitDeviceFeatures();
		virtual void ConfigureRenderer();
		virtual void InitLayouts();
		virtual void ConfigureShaders();
		
		// Scene
		virtual void ReadShaders();
		virtual void LoadScene();
		virtual void UnloadScene();
		
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
		
	public: // Public Render Methods
		virtual void Render();
	};
}

DEFINE_EVENT(v4d::graphics::renderer, PipelinesCreate, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, PipelinesDestroy, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Load, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Unload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Reload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Resize, Renderer*)
