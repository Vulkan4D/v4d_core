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
			computeQueue,  lowPriorityComputeQueue, 
			presentQueue,
			transferQueue;

		// Command buffers
		std::vector<VkCommandBuffer> graphicsCommandBuffers, computeCommandBuffers, graphicsDynamicCommandBuffers, computeDynamicCommandBuffers;
		VkCommandBuffer lowPriorityGraphicsCommandBuffer, lowPriorityComputeCommandBuffer;

		// Swap Chains
		SwapChain* swapChain = nullptr;

		// Synchronizations
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores, dynamicRenderFinishedSemaphores;
		std::vector<VkSemaphore> computeFinishedSemaphores, dynamicComputeFinishedSemaphores;
		std::vector<VkFence> graphicsFences, computeFences;
		size_t currentFrameInFlight = 0;
		const int MAX_FRAMES_IN_FLIGHT = 2;
		
		// States
		std::recursive_mutex renderingMutex, lowPriorityRenderingMutex;
		
		// Descriptor sets
		VkDescriptorPool descriptorPool;
		std::vector<DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};

		// Preferences
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
		
		// Resources
		virtual void CreateResources() = 0;
		virtual void DestroyResources() = 0;
		virtual void AllocateBuffers() = 0;
		virtual void FreeBuffers() = 0;
		
		// Scene
		virtual void LoadScene() = 0;
		virtual void UnloadScene() = 0;
		virtual void ReadShaders() = 0;
		
		// Pipelines
		virtual void CreatePipelines() = 0;
		virtual void DestroyPipelines() = 0;
		
		// Update
		virtual void FrameUpdate(uint imageIndex) = 0;
		virtual void LowPriorityFrameUpdate() = 0;
		
		// Commands
		virtual void RecordComputeCommandBuffer(VkCommandBuffer, int imageIndex) = 0;
		virtual void RecordGraphicsCommandBuffer(VkCommandBuffer, int imageIndex) = 0;
		virtual void RecordLowPriorityComputeCommandBuffer(VkCommandBuffer) = 0;
		virtual void RecordLowPriorityGraphicsCommandBuffer(VkCommandBuffer) = 0;
		virtual void RunDynamicCompute(VkCommandBuffer) = 0;
		virtual void RunDynamicGraphics(VkCommandBuffer) = 0;
		virtual void RunDynamicLowPriorityCompute(VkCommandBuffer) = 0;
		virtual void RunDynamicLowPriorityGraphics(VkCommandBuffer) = 0;

	protected: // Virtual INIT Methods

		virtual void CreateDevices();
		virtual void DestroyDevices();

		virtual void CreateSyncObjects();
		virtual void DestroySyncObjects();
		
		virtual void CreateCommandPools();
		virtual void DestroyCommandPools();
		
		virtual void CreateDescriptorSets();
		virtual void DestroyDescriptorSets();
		virtual void UpdateDescriptorSets();

		virtual void CreateSwapChain();
		virtual void DestroySwapChain();

		virtual void CreateCommandBuffers();
		virtual void DestroyCommandBuffers();

	protected: // Helper methods

		VkCommandBuffer BeginSingleTimeCommands(Queue queue);
		void EndSingleTimeCommands(Queue queue, VkCommandBuffer commandBuffer);

		void AllocateBufferStaged(Queue queue, Buffer& buffer);
		void AllocateBuffersStaged(Queue queue, std::vector<Buffer>& buffers);
		void AllocateBuffersStaged(Queue queue, std::vector<Buffer*>& buffers);

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


/* 
#include <v4d.h>

class MyRenderer : public v4d::graphics::Renderer {
	using v4d::graphics::Renderer::Renderer;
	
private: // Init
	void ScorePhysicalDeviceSelection(int& score, PhysicalDevice* physicalDevice) override {}
	void Init() override {}
	void Info() override {}
	void CreateLayouts() override {}
	void DestroyLayouts() override {}
	void ConfigureShaders() override {}

private: // Resources
	void CreateResources() override {}
	void DestroyResources() override {}
	void AllocateBuffers() override {}
	void FreeBuffers() override {}

private: // Graphics configuration
	void CreatePipelines() override {}
	void DestroyPipelines() override {}
	
private: // Commands
	void RecordComputeCommandBuffer(VkCommandBuffer, int imageIndex) override {}
	void RecordGraphicsCommandBuffer(VkCommandBuffer commandBuffer, int imageIndex) override {}
	void RecordLowPriorityComputeCommandBuffer(VkCommandBuffer) override {}
	void RecordLowPriorityGraphicsCommandBuffer(VkCommandBuffer) override {}
	void RunDynamicCompute(VkCommandBuffer) override {}
	void RunDynamicGraphics(VkCommandBuffer) override {}
	void RunDynamicLowPriorityCompute(VkCommandBuffer) override {}
	void RunDynamicLowPriorityGraphics(VkCommandBuffer) override {}
	
public: // Scene configuration
	void LoadScene() override {}
	void UnloadScene() override {}
	
public: // Update
	void FrameUpdate(uint imageIndex) override {}
	void LowPriorityFrameUpdate() override {}
};

*/
