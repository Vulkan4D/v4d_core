#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB BufferSrcDataPtr {
		void* dataPtr;
		size_t size;
		
		BufferSrcDataPtr(void* dataPtr, size_t size);
	};

	struct V4DLIB Buffer {
		// Mandatory fields
		VkBufferUsageFlags usage;
		VkDeviceSize size;
		bool alignedUniformSize;
		
		// Additional fields
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		// Allocated handles
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		
		// Mapped data
		void* data = nullptr;
		
		// Data pointers to get copied into buffer
		std::vector<BufferSrcDataPtr> srcDataPointers {};
		
		Buffer(VkBufferUsageFlags usage, VkDeviceSize size = 0, bool alignedUniformSize = false);
		
		void AddSrcDataPtr(void* srcDataPtr, size_t size);
		
		template<class T>
		void AddSrcDataPtr(std::vector<T>* vector) {
			AddSrcDataPtr(vector->data(), vector->size() * sizeof(T));
		}
		
		void AllocateFromStaging(Device* device, VkCommandBuffer commandBuffer, Buffer& stagingBuffer, VkDeviceSize size = 0, VkDeviceSize offset = 0);

		void Allocate(Device* device, VkMemoryPropertyFlags properties, bool copySrcData = true);
		void Free(Device* device);

		void MapMemory(Device* device, VkDeviceSize offset = 0, VkDeviceSize size = 0, VkMemoryMapFlags flags = 0);
		void UnmapMemory(Device* device);
		
		void WriteToMappedData(Device* device, void* inputData, size_t copySize = 0);
		void ReadFromMappedData(Device* device, void* outputData, size_t copySize = 0);
		
		// static void CopyDataToBuffer(Device* device, void* data, Buffer* buffer, VkDeviceSize offset = 0, VkDeviceSize size = 0, VkMemoryMapFlags flags = 0);
		// static void CopyDataToMappedBuffer(Device* device, void* inputData, Buffer* buffer, long mappedOffset = 0, size_t size = 0);
		
		static void Copy(Device* device, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
		static void Copy(Device* device, VkCommandBuffer commandBuffer, Buffer srcBuffer, Buffer dstBuffer, VkDeviceSize size = 0, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

	};
}
