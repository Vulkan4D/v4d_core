#include <v4d.h>

using namespace v4d::graphics::vulkan;

Buffer::Buffer(VkBufferUsageFlags usage, VkDeviceSize size, bool alignedUniformSize) : usage(usage), size(size), alignedUniformSize(alignedUniformSize) {}

void Buffer::AddSrcDataPtr(void* srcDataPtr, size_t size) {
	srcDataPointers.push_back(BufferSrcDataPtr(srcDataPtr, size));
}

void Buffer::ResetSrcData() {
	srcDataPointers.clear();
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

void Buffer::Allocate(Device* device, VkMemoryPropertyFlags properties, bool copySrcData, const std::vector<uint32_t>& queueFamilies) {
	this->properties = properties;
	if (queueFamilies.size() > 0) {
		sharingMode = VK_SHARING_MODE_CONCURRENT;
	}
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
	bufferInfo.queueFamilyIndexCount = queueFamilies.size();
	bufferInfo.pQueueFamilyIndices = queueFamilies.data();
	
	if (device->CreateBuffer(&bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	device->GetBufferMemoryRequirements(buffer, &memRequirements);
	
	VkDeviceSize allocSize = memRequirements.size;
	if ((allocSize % memRequirements.alignment) > 0) {
		allocSize += memRequirements.alignment - (allocSize % memRequirements.alignment);
	}
	
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo {};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR) {
			memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		}

	VkMemoryAllocateInfo allocInfo = {};
		allocInfo.pNext = memoryAllocateFlagsInfo.flags > 0 ? &memoryAllocateFlagsInfo : nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = allocSize;
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
		CopySrcData(device);
	}

	device->BindBufferMemory(buffer, memory, 0);
}

void Buffer::CopySrcData(Device* device, size_t maxCopySize) {
	bool autoMapMemory = !data;
	if (autoMapMemory) MapMemory(device);
	long offset = 0;
	if (maxCopySize == 0) maxCopySize = size;
	else maxCopySize = std::min(size, maxCopySize);
	for (auto& dataPointer : srcDataPointers) {
		size_t copySize = std::min(dataPointer.size, maxCopySize);
		memcpy((byte*)data + offset, dataPointer.dataPtr, copySize);
		maxCopySize -= copySize;
		offset += copySize;
		if (maxCopySize <= 0) break;
	}
	if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
		VkMappedMemoryRange mappedRange {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = 0;
		mappedRange.size = std::min(size, maxCopySize);
		device->FlushMappedMemoryRanges(1, &mappedRange);
	}
	if (autoMapMemory) UnmapMemory(device);
}

void Buffer::Free(Device* device) {
	if (buffer != VK_NULL_HANDLE) {
		device->DestroyBuffer(buffer, nullptr);
		if (memory != VK_NULL_HANDLE) device->FreeMemory(memory, nullptr);
		buffer = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
		data = nullptr;
	}
	if (srcDataPointers.size() > 0) {
		size = 0;
	}
}

void Buffer::MapMemory(Device* device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) {
	device->MapMemory(memory, offset, size == 0 ? this->size : size, flags, &data);
}

void Buffer::UnmapMemory(Device* device) {
	if (data) device->UnmapMemory(memory);
	data = nullptr;
}

void Buffer::WriteToMappedData(void* inputData, size_t copySize, size_t deviceMemoryOffset) {
	memcpy((byte*)data + deviceMemoryOffset, inputData, copySize == 0 ? size : copySize);
}

void Buffer::ReadFromMappedData(void* outputData, size_t copySize, size_t deviceMemoryOffset) {
	memcpy(outputData, (byte*)data + deviceMemoryOffset, copySize == 0 ? size : copySize);
}

void Buffer::Copy(Device* device, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;
	device->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void Buffer::Copy(Device* device, VkCommandBuffer commandBuffer, Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size == 0 ? srcBuffer.size : size;
	device->CmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
}

BufferSrcDataPtr::BufferSrcDataPtr(void* dataPtr, size_t size) : dataPtr(dataPtr), size(size) {}


// struct StagedBuffer

StagedBuffer::StagedBuffer(VkBufferUsageFlags usage, VkDeviceSize size, bool alignedUniformSize)
: stagingBuffer({usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size, alignedUniformSize}), deviceLocalBuffer({usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size, alignedUniformSize}) {}

void StagedBuffer::AddSrcDataPtr(void* srcDataPtr, size_t size) {
	stagingBuffer.AddSrcDataPtr(srcDataPtr, size);
	deviceLocalBuffer.AddSrcDataPtr(srcDataPtr, size);
}

void StagedBuffer::ResetSrcData() {
	stagingBuffer.ResetSrcData();
	deviceLocalBuffer.ResetSrcData();
}

void StagedBuffer::Allocate(Device* device, VkMemoryPropertyFlags properties, const std::vector<uint32_t>& queueFamilies) {
	stagingBuffer.Allocate(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false, queueFamilies);
	stagingBuffer.MapMemory(device);
	deviceLocalBuffer.Allocate(device, properties, false, queueFamilies);
}

void StagedBuffer::Free(Device* device) {
	stagingBuffer.UnmapMemory(device);
	stagingBuffer.Free(device);
	deviceLocalBuffer.Free(device);
}

void StagedBuffer::Update(Device* device, VkCommandBuffer commandBuffer, size_t maxCopySize) {
	stagingBuffer.CopySrcData(device, maxCopySize);
	Buffer::Copy(device, commandBuffer, stagingBuffer, deviceLocalBuffer, maxCopySize);
}
