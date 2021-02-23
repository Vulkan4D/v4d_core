#pragma once
#include <v4d.h>

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
	
	struct RayCast {
		uint64_t moduleVen = 0;
		uint64_t moduleId = 0;
		uint64_t objId = 0;
		uint64_t raycastCustomData = 0;
		glm::vec3 localSpaceHitPosition;
		glm::f32 distance;
		glm::vec4 localSpaceHitSurfaceNormal; // w component is unused
		
		bool operator==(const RayCast& other) const {
			return moduleVen == other.moduleVen && moduleId == other.moduleId && objId == other.objId;
		}
		bool operator!=(const RayCast& other) const {
			return !(*this == other);
		}
		operator bool () const {
			return moduleVen && moduleId;
		}
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
		uint64_t frameIndex = 0;
		double deltaTime = 1.0/60;
		double avgDeltaTime = 1.0/60;
		
		// Descriptor sets
		VkDescriptorPool descriptorPool;
		std::map<std::string, DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		
		// Ray-Tracing Shaders
		static std::unordered_map<std::string, uint32_t> sbtOffsets;
		
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
			// {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
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
		
		void RequiredDeviceExtension(const char* ext);
		void OptionalDeviceExtension(const char* ext);
		bool IsDeviceExtensionEnabled(const char* ext);

	public: // Virtual methods
		// Init
		virtual void InitDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* supportedDeviceFeatures);
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

		void TransitionImageLayout(VkCommandBuffer commandBuffer, Image image, VkImageLayout oldLayout, VkImageLayout newLayout);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, VkImageAspectFlags aspectMask = 0);

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
		virtual void Update(double deltaTime);
	};
}

DEFINE_EVENT(v4d::graphics::renderer, PipelinesCreate, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, PipelinesDestroy, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Load, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Unload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Reload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Resize, Renderer*)
