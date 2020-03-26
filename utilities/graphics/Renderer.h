#pragma once
#include <v4d.h>

namespace v4d::graphics {
	using namespace v4d::graphics::vulkan;

	class V4DLIB Renderer : public Instance {
	protected: // class members

		// Main Render Surface
		VkSurfaceKHR surface;

		// Main Graphics Card
		PhysicalDevice* renderingPhysicalDevice = nullptr;
		Device* renderingDevice = nullptr;
		
		// Queues
		Queue graphicsQueue, lowPriorityGraphicsQueue, 
			#ifdef V4D_RENDERER_MAIN_COMPUTE_COMMANDS_ENABLED
				computeQueue,  
			#endif
			lowPriorityComputeQueue, 
			presentQueue,
			transferQueue;

		// Command buffers
		std::vector<VkCommandBuffer> graphicsCommandBuffers, graphicsDynamicCommandBuffers;
		#ifdef V4D_RENDERER_MAIN_COMPUTE_COMMANDS_ENABLED
			std::vector<VkCommandBuffer> computeCommandBuffers, computeDynamicCommandBuffers;
		#endif
		#if V4D_RENDERER_LOW_PRIORITY_RECORDED_COMMANDS_ENABLED
			VkCommandBuffer lowPriorityGraphicsCommandBuffer, lowPriorityComputeCommandBuffer;
		#endif

		// Swap Chains
		SwapChain* swapChain = nullptr;

		// Synchronizations
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> staticRenderFinishedSemaphores;
		std::vector<VkSemaphore> dynamicRenderFinishedSemaphores;
		#ifdef V4D_RENDERER_MAIN_COMPUTE_COMMANDS_ENABLED
			std::vector<VkSemaphore> computeFinishedSemaphores;
			std::vector<VkSemaphore> dynamicComputeFinishedSemaphores;
		#endif
		std::vector<VkFence> graphicsFences;
		std::vector<VkFence> computeFences;
		size_t currentFrameInFlight = 0;
		const int NB_FRAMES_IN_FLIGHT = 2;
		
		// States
		std::recursive_mutex renderingMutex, lowPriorityRenderingMutex;
		bool mustReload = false;
		bool graphicsLoadedToDevice = false;
		std::thread::id renderThreadId = std::this_thread::get_id();
		
		// Descriptor sets
		VkDescriptorPool descriptorPool;
		std::map<std::string, DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};

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

	private: // Device Extensions and features
		std::vector<const char*> requiredDeviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<const char*> optionalDeviceExtensions {};
		std::vector<const char*> deviceExtensions {};
		std::unordered_map<std::string, bool> enabledDeviceExtensions {};
		
	public: 
		VkPhysicalDeviceFeatures deviceFeatures {}; // This object will be modified to keep only the enabled values.
		VkPhysicalDeviceVulkan12Features vulkan12DeviceFeatures {};
		
		void RequiredDeviceExtension(const char* ext);
		void OptionalDeviceExtension(const char* ext);
		bool IsDeviceExtensionEnabled(const char* ext);

	protected: // Virtual methods
		// Init
		virtual void ScorePhysicalDeviceSelection(int& score, PhysicalDevice* physicalDevice) = 0;
		virtual void Init() = 0;
		virtual void Info() = 0;
		virtual void InitLayouts() = 0;
		virtual void ConfigureShaders() = 0;
		
		// Scene
		virtual void ReadShaders() = 0;
		virtual void LoadScene() = 0;
		virtual void UnloadScene() = 0;
		
		// Resources
		virtual void CreateResources() = 0;
		virtual void DestroyResources() = 0;
		virtual void AllocateBuffers() = 0;
		virtual void FreeBuffers() = 0;
		
		// Pipelines
		virtual void CreatePipelines() = 0;
		virtual void DestroyPipelines() = 0;
		
		// Update
		virtual void FrameUpdate(uint imageIndex) = 0;
		virtual void LowPriorityFrameUpdate() = 0;
		virtual void BeforeGraphics() {};
		
		// Commands
		virtual void RunDynamicGraphics(VkCommandBuffer) = 0;
		virtual void RecordGraphicsCommandBuffer(VkCommandBuffer, int imageIndex) = 0;
		virtual void RunDynamicLowPriorityCompute(VkCommandBuffer) = 0;
		virtual void RunDynamicLowPriorityGraphics(VkCommandBuffer) = 0;
		
		#if V4D_RENDERER_LOW_PRIORITY_RECORDED_COMMANDS_ENABLED
			virtual void RecordLowPriorityComputeCommandBuffer(VkCommandBuffer) {}
			virtual void RecordLowPriorityGraphicsCommandBuffer(VkCommandBuffer) {}
		#endif
		#ifdef V4D_RENDERER_MAIN_COMPUTE_COMMANDS_ENABLED
			virtual void RecordComputeCommandBuffer(VkCommandBuffer, int imageIndex) {}
			virtual void RunDynamicCompute(VkCommandBuffer) {}
		#endif

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
	
		VkCommandBuffer BeginSingleTimeCommands(Queue queue);
		void EndSingleTimeCommands(Queue queue, VkCommandBuffer commandBuffer);

		void AllocateBufferStaged(Queue queue, Buffer& buffer);
		void AllocateBuffersStaged(Queue queue, std::vector<Buffer>& buffers);
		void AllocateBuffersStaged(Queue queue, std::vector<Buffer*>& buffers);
		
		void AllocateBufferStaged(Buffer& buffer);
		void AllocateBuffersStaged(std::vector<Buffer>& buffers);
		void AllocateBuffersStaged(std::vector<Buffer*>& buffers);

		void TransitionImageLayout(Image image, VkImageLayout oldLayout, VkImageLayout newLayout);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, Image image, VkImageLayout oldLayout, VkImageLayout newLayout);
		void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		// void GenerateMipmaps(Texture2D* texture);

		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels);

	protected: // Init/Reset Methods
		virtual void RecreateSwapChains();
		
	public: // Init/Load/Reset Methods
		virtual void InitRenderer();
		virtual void LoadRenderer();
		virtual void UnloadRenderer();
		virtual void ReloadRenderer();
		
	protected:
		virtual void LoadGraphicsToDevice();
		virtual void UnloadGraphicsFromDevice();
		
	public: // Constructor & Destructor
		Renderer(Loader* loader, const char* applicationName, uint applicationVersion, Window* window);
		virtual ~Renderer() override;
		
	public: // Public Render Methods
		virtual void Render();
		virtual void RenderLowPriority();
	};
}

DEFINE_EVENT(v4d::graphics::renderer, PipelinesCreate, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, PipelinesDestroy, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Load, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Unload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Reload, Renderer*)
DEFINE_EVENT(v4d::graphics::renderer, Resize, Renderer*)


/* 
#include <v4d.h>

class MyRenderer : public v4d::graphics::Renderer {
	using v4d::graphics::Renderer::Renderer;
	
private: // Init
	void ScorePhysicalDeviceSelection(int& score, PhysicalDevice* physicalDevice) override {}
	void Init() override {}
	void Info() override {}
	void InitLayouts() override {}
	void ConfigureShaders() override {}

public: // Scene configuration
	void ReadShaders() override {}
	void LoadScene() override {}
	void UnloadScene() override {}
	
private: // Resources
	void CreateResources() override {}
	void DestroyResources() override {}
	void AllocateBuffers() override {}
	void FreeBuffers() override {}

private: // Graphics configuration
	void CreatePipelines() override {}
	void DestroyPipelines() override {}
	
public: // Update
	void FrameUpdate(uint imageIndex) override {}
	void LowPriorityFrameUpdate() override {}
	void BeforeGraphics() override {}
	
private: // Commands
	void RunDynamicGraphics(VkCommandBuffer commandBuffer) override {}
	void RecordGraphicsCommandBuffer(VkCommandBuffer commandBuffer, int imageIndex) override {}
	void RunDynamicLowPriorityCompute(VkCommandBuffer commandBuffer) override {}
	void RunDynamicLowPriorityGraphics(VkCommandBuffer commandBuffer) override {}
	
*/
