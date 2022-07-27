#include "MemoryPool.h"
namespace v4d::graphics::vulkan {
	MemoryPool::MemoryPool(Device* device, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaPoolCreateInfo poolInfo) : device(device) {
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = 1024; // Size doesn't matter here
		bufferCreateInfo.usage = bufferUsage;
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = memoryUsage;
		vmaFindMemoryTypeIndexForBufferInfo(device->GetAllocator(), &bufferCreateInfo, &allocCreateInfo, &poolInfo.memoryTypeIndex);
		if (vmaCreatePool(device->GetAllocator(), &poolInfo, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create memory pool");
		}
	}
	MemoryPool::~MemoryPool() {
		vmaDestroyPool(device->GetAllocator(), handle);
	}
}
