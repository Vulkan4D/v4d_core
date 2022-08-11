/*
 * Vulkan Logical Device abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 */
#pragma once

#include <v4d.h>
#include <map>
#include <unordered_map>
#include <mutex>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/PhysicalDevice.h"
#include "utilities/graphics/vulkan/Queue.hpp"

#ifndef V4D_DEFERRED_DEALLOCATION_MAX_COUNT
	#define V4D_DEFERRED_DEALLOCATION_MAX_COUNT 4096
#endif

namespace v4d::graphics::vulkan {

	struct VertexInputAttributeDescription {
		uint32_t location;
		uint32_t offset;
		VkFormat format;
	};

	enum MemoryUsage {
		/** No intended memory usage specified.
		*/
		MEMORY_USAGE_UNKNOWN = 0,
		
		
		/** Memory will be used on device only, so fast access from the device is preferred.
		It usually means device-local GPU (video) memory.
		No need to be mappable on host.
		It is roughly equivalent of `D3D12_HEAP_TYPE_DEFAULT`.

		Usage:

		- Resources written and read by device, e.g. images used as attachments.
		- Resources transferred from host once (immutable) or infrequently and read by
		device multiple times, e.g. textures to be sampled, vertex buffers, uniform
		(constant) buffers, and majority of other types of resources used on GPU.

		Allocation may still end up in `HOST_VISIBLE` memory on some implementations.
		In such case, you are free to map it.
		You can use #VMA_ALLOCATION_CREATE_MAPPED_BIT with this usage type.
		*/
		MEMORY_USAGE_GPU_ONLY = 1,
		
		
		/** Memory will be mappable on host.
		It usually means CPU (system) memory.
		Guarantees to be `HOST_VISIBLE` and `HOST_COHERENT`.
		CPU access is typically uncached. Writes may be write-combined.
		Resources created in this pool may still be accessible to the device, but access to them can be slow.
		It is roughly equivalent of `D3D12_HEAP_TYPE_UPLOAD`.

		Usage: Staging copy of resources used as transfer source.
		*/
		MEMORY_USAGE_CPU_ONLY = 2,
		
		
		/**
		Memory that is both mappable on host (guarantees to be `HOST_VISIBLE`) and preferably fast to access by GPU.
		CPU access is typically uncached. Writes may be write-combined.

		Usage: Resources written frequently by host (dynamic), read by device. E.g. textures (with LINEAR layout), vertex buffers, uniform buffers updated every frame or every draw call.
		*/
		MEMORY_USAGE_CPU_TO_GPU = 3,
		
		
		/** Memory mappable on host (guarantees to be `HOST_VISIBLE`) and cached.
		It is roughly equivalent of `D3D12_HEAP_TYPE_READBACK`.

		Usage:

		- Resources written by device, read by host - results of some computations, e.g. screen capture, average scene luminance for HDR tone mapping.
		- Any resources read or accessed randomly on host, e.g. CPU-side copy of vertex buffer used as source of transfer, but also used for collision detection.
		*/
		MEMORY_USAGE_GPU_TO_CPU = 4,
		
		
		/** CPU memory - memory that is preferably not `DEVICE_LOCAL`, but also not guaranteed to be `HOST_VISIBLE`.

		Usage: Staging copy of resources moved from GPU memory to CPU memory as part
		of custom paging/residency mechanism, to be moved back to GPU memory when needed.
		*/
		MEMORY_USAGE_CPU_COPY = 5,
		
		
		/** Lazily allocated GPU memory having `VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`.
		Exists mostly on mobile platforms. Using it on desktop PC or other GPUs with no such memory type present will fail the allocation.

		Usage: Memory for transient attachment images (color attachments, depth attachments etc.), created with `VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT`.

		Allocations with this usage are always created as dedicated - it implies #VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT.
		*/
		MEMORY_USAGE_GPU_LAZILY_ALLOCATED = 6,

		MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF
	};
	
	typedef VmaAllocation MemoryAllocation;

	class V4DLIB Device : public xvk::Interface::DeviceInterface {
	private:
		PhysicalDevice* physicalDevice;
		VkDeviceCreateInfo createInfo {};
	
		VmaAllocator allocator;
		
	public:
		Device(
			PhysicalDevice* physicalDevice,
			std::vector<const char*>& extensions,
			std::vector<const char*>& layers,
			std::map<VkQueueFlags, std::map<uint, Queue>>& queues,
			void* pNext = nullptr
		);
		~Device();
		
		void DumpMemoryAllocationStats();
		VmaAllocator GetAllocator() const {return allocator;}

		VkDevice GetHandle() const;
		VkPhysicalDevice GetPhysicalDeviceHandle() const;
		PhysicalDevice* GetPhysicalDevice() const;

		std::map<VkQueueFlags, std::map<uint, Queue>>& queues;
		
		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::CreateCommandPool;
		void CreateCommandPool(uint queueIndex, VkCommandPoolCreateFlags flags, VkCommandPool* commandPool);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::DestroyCommandPool;
		void DestroyCommandPool(VkCommandPool&);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::CreateDescriptorPool;
		void CreateDescriptorPool(std::vector<VkDescriptorType> types, uint32_t count, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);
		void CreateDescriptorPool(std::map<VkDescriptorType, uint>& types, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::DestroyDescriptorPool;
		void DestroyDescriptorPool(VkDescriptorPool&);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::GetBufferDeviceAddress;
		VkDeviceAddress GetBufferDeviceAddress(const VkBuffer&);
		VkDeviceOrHostAddressKHR GetBufferDeviceOrHostAddress(const VkBuffer&);
		VkDeviceOrHostAddressConstKHR GetBufferDeviceOrHostAddressConst(const VkBuffer&);

		// Helpers
		size_t GetAlignedUniformSize(size_t size);

		VkCommandBuffer BeginSingleTimeCommands(Queue, uint commandPoolIndex = 0);
		void EndSingleTimeCommands(Queue, VkCommandBuffer, uint commandPoolIndex = 0);
		void RunSingleTimeCommands(Queue, std::function<void(VkCommandBuffer)>&&, uint commandPoolIndex = 0);
		bool TryRunSingleTimeCommands(Queue, std::function<bool(VkCommandBuffer)>&&, uint commandPoolIndex = 0);
		
		// Allocator
		void CreateAllocator();
		void DestroyAllocator();
		VkResult CreateAndAllocateBuffer(const VkBufferCreateInfo&, VmaPool, VkBuffer&, MemoryAllocation*);
		VkResult CreateAndAllocateBuffer(const VkBufferCreateInfo&, MemoryUsage, VkBuffer&, MemoryAllocation*, bool weakAllocation = false);
		VkResult CreateAndAllocateImage(const VkImageCreateInfo&, MemoryUsage, VkImage&, MemoryAllocation*, bool weakAllocation = true);
		void FreeAndDestroyBuffer(VkBuffer&, MemoryAllocation&, uint64_t frameIndex = 0);
		void FreeAndDestroyImage(VkImage&, MemoryAllocation&, uint64_t frameIndex = 0);
		VkResult MapMemoryAllocation(MemoryAllocation&, void** data, VkDeviceSize offset = 0, VkDeviceSize size = 0);
		void UnmapMemoryAllocation(MemoryAllocation&);
		VkResult FlushMemoryAllocation(MemoryAllocation&, VkDeviceSize offset, VkDeviceSize size);
		void AllocatorSetCurrentFrameIndex(uint32_t);
		bool TouchAllocation(MemoryAllocation&);
		
		// Debug Utils
		inline void SetDebugName(VkObjectType type, uint64_t obj, const char* name) {
			#ifdef _DEBUG
				VkDebugUtilsObjectNameInfoEXT debugNameInfo {
					VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					nullptr, type, obj, name
				};
				SetDebugUtilsObjectNameEXT(&debugNameInfo);
			#endif
		}
		#ifdef _DEBUG
			#define __V4D_DECLARE_SET_DEBUG_NAME_FUNC(Type, ObjectTypeFlag) inline void SetDebugName(Type obj, const char* name) {SetDebugName(ObjectTypeFlag, uint64_t(obj), name);}
		#else
			#define __V4D_DECLARE_SET_DEBUG_NAME_FUNC(Type, ObjectTypeFlag) inline void SetDebugName(Type obj, const char* name) {}
		#endif
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkInstance, VK_OBJECT_TYPE_INSTANCE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkPhysicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDevice, VK_OBJECT_TYPE_DEVICE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkQueue, VK_OBJECT_TYPE_QUEUE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkFence, VK_OBJECT_TYPE_FENCE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkBuffer, VK_OBJECT_TYPE_BUFFER)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkImage, VK_OBJECT_TYPE_IMAGE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkEvent, VK_OBJECT_TYPE_EVENT)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkPipelineCache, VK_OBJECT_TYPE_PIPELINE_CACHE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkSampler, VK_OBJECT_TYPE_SAMPLER)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkSamplerYcbcrConversion, VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDescriptorUpdateTemplate, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkSurfaceKHR, VK_OBJECT_TYPE_SURFACE_KHR)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkSwapchainKHR, VK_OBJECT_TYPE_SWAPCHAIN_KHR)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDisplayKHR, VK_OBJECT_TYPE_DISPLAY_KHR)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDisplayModeKHR, VK_OBJECT_TYPE_DISPLAY_MODE_KHR)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkDebugUtilsMessengerEXT, VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT)
		__V4D_DECLARE_SET_DEBUG_NAME_FUNC(VkAccelerationStructureKHR, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR)
		
		
		struct DeferredDeallocation {
			std::atomic<bool> lock = false;
			VmaAllocation allocation = 0;
			uint64_t frameIndex = 0;
			VkBuffer buffer = 0;
			VkImage img = 0;
		};
		template<typename T>
		inline static void DeferDeallocation(const T& obj, const VmaAllocation& allocation, const uint64_t& frameIndex) {
			int nbTries = 0;
			DeferredDeallocation* deferredDealloc = nullptr;
			do {
				assert(++nbTries < V4D_DEFERRED_DEALLOCATION_MAX_COUNT);
				if (deferredDealloc) deferredDealloc->lock = false;
				deferredDealloc = GetNextDeferredDeallocation();
			} while(deferredDealloc->allocation);
			if constexpr (std::is_same_v<T, VkBuffer>) {
				deferredDealloc->buffer = obj;
			} else if constexpr (std::is_same_v<T, VkImage>) {
				deferredDealloc->img = obj;
			}
			deferredDealloc->frameIndex = frameIndex;
			deferredDealloc->allocation = allocation;
			deferredDealloc->lock = false;
		}
		void DestroyDeferredDeallocations(bool = false);
		static void SetDeferredDeallocationCurrentFrame(uint64_t frameIndex);
	private:
		static DeferredDeallocation* GetNextDeferredDeallocation();
		static uint64_t deferredDeallocationCurrentFrame;
		static std::atomic<uint64_t> nextDeferredDeallocationIndex;
		static std::array<DeferredDeallocation, V4D_DEFERRED_DEALLOCATION_MAX_COUNT> deferredDeallocations;

	};
}
