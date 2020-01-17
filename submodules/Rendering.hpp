#pragma once

namespace v4d::modules {
	using namespace v4d::graphics;
	using namespace v4d::graphics::vulkan;
	
	class Rendering {
	public: 
		V4D_DEFINE_SUBMODULE( SUBMODULE_TYPE_GRAPHICS | 1 )
		Rendering() {}
		virtual ~Rendering() {}
		
	protected: 
		Renderer* renderer = nullptr;
		Device* renderingDevice = nullptr;
		Queue* graphicsQueue = nullptr;
		Queue* lowPriorityGraphicsQueue = nullptr;
		Queue* lowPriorityComputeQueue = nullptr;
		Queue* transferQueue = nullptr;
		SwapChain* swapChain = nullptr;
		
	public:
		void SetRenderer(Renderer* renderer) {
			this->renderer = renderer;
		}
		void SetRenderingDevice(Device* renderingDevice) {
			this->renderingDevice = renderingDevice;
		}
		void SetGraphicsQueue(Queue* queue) {
			this->graphicsQueue = queue;
		}
		void SetLowPriorityGraphicsQueue(Queue* queue) {
			this->lowPriorityGraphicsQueue = queue;
		}
		void SetLowPriorityComputeQueue(Queue* queue) {
			this->lowPriorityComputeQueue = queue;
		}
		void SetTransferQueue(Queue* queue) {
			this->transferQueue = queue;
		}
		void SetSwapChain(SwapChain* swapChain) {
			this->swapChain = swapChain;
		}
		
	public: // Methods that are typically overridden, in the correct execution order
	
		// Executed when calling InitRenderer() on the main Renderer
		virtual void Init() {}
		virtual void InitLayouts(std::vector<DescriptorSet*>&) {}
		virtual void ConfigureShaders(std::unordered_map<std::string, std::vector<RasterShaderPipeline*>>& shaders) {}
		
		// Executed when calling their respective methods on the main Renderer
		virtual void ReadShaders() {}
		virtual void LoadScene() {}
		virtual void UnloadScene() {}
		
		// Executed when calling LoadRenderer()
		virtual void ScorePhysicalDeviceSelection(int& score, PhysicalDevice*) {}
		// after selecting rendering device and queues
		virtual void Info() {}
		virtual void CreateResources() {} // here we have the swapChain available
		virtual void DestroyResources() {}
		virtual void AllocateBuffers() {}
		virtual void FreeBuffers() {}
		virtual void CreatePipelines() {}
		virtual void DestroyPipelines() {}
		
		// Rendering methods potentially executed on each frame
		virtual void RunDynamicGraphicsTop(VkCommandBuffer) {}
		virtual void RunDynamicGraphicsBottom(VkCommandBuffer) {}
		virtual void RunDynamicLowPriorityCompute(VkCommandBuffer) {}
		virtual void RunDynamicLowPriorityGraphics(VkCommandBuffer) {}
		
		// Executed before each frame
		virtual void FrameUpdate(uint imageIndex, glm::dmat4& projection, glm::dmat4& view) {}
		virtual void LowPriorityFrameUpdate() {}
		
	};
}
