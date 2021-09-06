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
	
	#ifdef V4D_VULKAN_USE_VMA
		typedef VmaAllocation MemoryAllocation;
	#else
		typedef VkDeviceMemory MemoryAllocation;
	#endif

	class V4DLIB Device : public xvk::Interface::DeviceInterface {
	private:
		PhysicalDevice* physicalDevice;
		VkDeviceCreateInfo createInfo {};
		
		#ifdef V4D_VULKAN_USE_VMA
			VmaAllocator allocator;
			std::mutex allocatorDeleteMutex;
		#endif

	public:
		Device(
			PhysicalDevice* physicalDevice,
			std::vector<const char*>& extensions,
			std::vector<const char*>& layers,
			std::map<VkQueueFlags, std::map<uint, Queue>>& queues,
			void* pNext = nullptr
		);
		~Device();
		
		#ifdef V4D_VULKAN_USE_VMA
			void DumpMemoryAllocationStats();
		#endif

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
		VkResult CreateAndAllocateBuffer(const VkBufferCreateInfo&, MemoryUsage, VkBuffer&, MemoryAllocation*, bool weakAllocation = false);
		VkResult CreateAndAllocateImage(const VkImageCreateInfo&, MemoryUsage, VkImage&, MemoryAllocation*, bool weakAllocation = true);
		void FreeAndDestroyBuffer(VkBuffer&, MemoryAllocation&);
		void FreeAndDestroyImage(VkImage&, MemoryAllocation&);
		VkResult MapMemoryAllocation(MemoryAllocation&, void** data, VkDeviceSize offset = 0, VkDeviceSize size = 0);
		void UnmapMemoryAllocation(MemoryAllocation&);
		VkResult FlushMemoryAllocation(MemoryAllocation&, VkDeviceSize offset, VkDeviceSize size);
		void AllocatorSetCurrentFrameIndex(uint32_t);
		bool TouchAllocation(MemoryAllocation&);
		
	};
}
