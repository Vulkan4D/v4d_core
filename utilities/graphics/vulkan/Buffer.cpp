#include <v4d.h>

using namespace v4d::graphics::vulkan;

Buffer::Buffer(VkBufferUsageFlags usage, VkDeviceSize size, bool alignedUniformSize) : usage(usage), size(size), alignedUniformSize(alignedUniformSize) {}

void Buffer::AddSrcDataPtr(void* srcDataPtr, size_t size) {
	srcDataPointers.push_back(BufferSrcDataPtr(srcDataPtr, size));
}

void Buffer::AllocateFromStaging(Device* device, VkCommandBuffer commandBuffer, Buffer& stagingBuffer, VkDeviceSize size, VkDeviceSize offset) {
	if (stagingBuffer.buffer == VK_NULL_HANDLE)
		throw std::runtime_error("Staging buffer is not allocated");
	if (size == 0 && offset == 0) size = stagingBuffer.size;
	this->size = size;
	usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
	Copy(device, commandBuffer, stagingBuffer.buffer, buffer, size, offset);
}

void Buffer::Allocate(Device* device, VkMemoryPropertyFlags properties, bool copySrcData) {
	if (size == 0) {
		for (auto& dataPointer : srcDataPointers) {
			size += dataPointer.size;
		}
	}
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = alignedUniformSize? device->GetAlignedUniformSize(size) : size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;
	
	if (device->CreateBuffer(&bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	device->GetBufferMemoryRequirements(buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->GetPhysicalDevice()->FindMemoryType(memRequirements.memoryTypeBits, properties);

	//TODO
	// It should be noted that in a real world application, we're not supposed to actually call vkAllocateMemory for every individual buffer-> 
	// The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even with high end hardware like a GTX 1080.
	// The right way to allocate memory for a large number of objects at the same time is to create a custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions.
	// We can either implement such an allocator ourselves, or use the VulkanMemoryAllocator library provided by the PhysicalDeviceOpen initiative.
	
	if (device->AllocateMemory(&allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory");
	}
	
	if (copySrcData && srcDataPointers.size() > 0) {
		MapMemory(device);
		long offset = 0;
		for (auto& dataPointer : srcDataPointers) {
			memcpy((byte*)data + offset, dataPointer.dataPtr, dataPointer.size);
			offset += dataPointer.size;
		}
		if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
			VkMappedMemoryRange mappedRange {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = 0;
			mappedRange.size = size;
			device->FlushMappedMemoryRanges(1, &mappedRange);
		}
		UnmapMemory(device);
	}

	device->BindBufferMemory(buffer, memory, 0);
}

void Buffer::Free(Device* device) {
	device->DestroyBuffer(buffer, nullptr);
	device->FreeMemory(memory, nullptr);
	buffer = VK_NULL_HANDLE;
	data = nullptr;
}

void Buffer::MapMemory(Device* device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) {
	device->MapMemory(memory, offset, size == 0 ? this->size : size, flags, &data);
}

void Buffer::UnmapMemory(Device* device) {
	device->UnmapMemory(memory);
	data = nullptr;
}

// void Buffer::CopyDataToBuffer(Device* device, void* data, Buffer* buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) {
// 	buffer->MapMemory(device, offset, size, flags);
// 	memcpy(buffer->data, data, size == 0 ? buffer->size : size);
// 	buffer->UnmapMemory(device);
// }

// void Buffer::CopyDataToMappedBuffer(Device* device, void* inputData, Buffer* buffer, long mappedOffset, size_t size) {
// 	memcpy(buffer->data + mappedOffset, inputData, size == 0 ? (buffer->size - mappedOffset) : size);
// }

void Buffer::WriteToMappedData(Device* device, void* inputData, size_t copySize) {
	memcpy(data, inputData, copySize == 0 ? size : copySize);
}

void Buffer::ReadFromMappedData(Device* device, void* outputData, size_t copySize) {
	memcpy(outputData, data, copySize == 0 ? size : copySize);
}


void Buffer::Copy(Device* device, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;
	device->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void Buffer::Copy(Device* device, VkCommandBuffer commandBuffer, Buffer srcBuffer, Buffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size == 0 ? srcBuffer.size : size;
	device->CmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
}

BufferSrcDataPtr::BufferSrcDataPtr(void* dataPtr, size_t size) : dataPtr(dataPtr), size(size) {}
