#include <v4d.h>

using namespace v4d::graphics::vulkan;

Buffer::Buffer(VkBufferUsageFlags usage, VkDeviceSize size, bool alignedUniformSize) : usage(usage), size(size), alignedUniformSize(alignedUniformSize) {}

void Buffer::ExtendSize(VkDeviceSize additionalSize) {
	if (allocation) {
		throw std::runtime_error("Cannot extend buffer size because it has already been allocated");
	}
	size += additionalSize;
}

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
	Allocate(device, MEMORY_USAGE_GPU_ONLY, false);
	Copy(device, commandBuffer, stagingBuffer.buffer, buffer, size, offset);
}

void Buffer::Allocate(Device* device, MemoryUsage memoryUsage, bool copySrcData, const std::vector<uint32_t>& queueFamilies, bool weakAllocation) {
	this->memoryUsage = memoryUsage;
	if (queueFamilies.size() > 0) {
		sharingMode = VK_SHARING_MODE_CONCURRENT;
	}
	if (size == 0) {
		for (auto& dataPointer : srcDataPointers) {
			size += dataPointer.size;
		}
	}
	
	VkBufferCreateInfo bufferInfo {};{
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = alignedUniformSize? device->GetAlignedUniformSize(size) : size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharingMode;
		bufferInfo.queueFamilyIndexCount = queueFamilies.size();
		bufferInfo.pQueueFamilyIndices = queueFamilies.data();
	}
	
	device->CreateAndAllocateBuffer(bufferInfo, memoryUsage, buffer, &allocation, weakAllocation);
	
	if (copySrcData && srcDataPointers.size() > 0) {
		CopySrcData(device);
	}
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
	if (memoryUsage != MEMORY_USAGE_CPU_ONLY) {
		device->FlushMemoryAllocation(allocation, 0, std::min(size, maxCopySize));
	}
	if (autoMapMemory) UnmapMemory(device);
}

void Buffer::Free(Device* device) {
	if (buffer != VK_NULL_HANDLE) {
		device->FreeAndDestroyBuffer(buffer, allocation);
		data = nullptr;
	}
	if (srcDataPointers.size() > 0) {
		size = 0;
	}
}

void Buffer::MapMemory(Device* device, VkDeviceSize offset, VkDeviceSize size) {
	device->MapMemoryAllocation(allocation, &data, offset, size == 0 ? this->size : size);
}

void Buffer::UnmapMemory(Device* device) {
	if (data) device->UnmapMemoryAllocation(allocation);
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

void StagedBuffer::ExtendSize(VkDeviceSize additionalSize) {
	stagingBuffer.ExtendSize(additionalSize);
	deviceLocalBuffer.ExtendSize(additionalSize);
}

void StagedBuffer::AddSrcDataPtr(void* srcDataPtr, size_t size) {
	stagingBuffer.AddSrcDataPtr(srcDataPtr, size);
	deviceLocalBuffer.AddSrcDataPtr(srcDataPtr, size);
}

void StagedBuffer::ResetSrcData() {
	stagingBuffer.ResetSrcData();
	deviceLocalBuffer.ResetSrcData();
}

void StagedBuffer::Allocate(Device* device, MemoryUsage memoryUsage, const std::vector<uint32_t>& queueFamilies, bool weakAllocation) {
	stagingBuffer.Allocate(device, MEMORY_USAGE_CPU_ONLY, false, queueFamilies, false);
	stagingBuffer.MapMemory(device);
	deviceLocalBuffer.Allocate(device, memoryUsage, false, queueFamilies, weakAllocation);
}

void StagedBuffer::Free(Device* device) {
	stagingBuffer.UnmapMemory(device);
	stagingBuffer.Free(device);
	deviceLocalBuffer.Free(device);
}

void StagedBuffer::Update(Device* device, VkCommandBuffer commandBuffer, size_t maxCopySize) {
	stagingBuffer.CopySrcData(device, maxCopySize);
	if (!device->TouchAllocation(deviceLocalBuffer.allocation)) {
		LOG_DEBUG("Staging Buffer Update() ALLOCATION LOST")
		return;
	}
	Buffer::Copy(device, commandBuffer, stagingBuffer, deviceLocalBuffer, maxCopySize);
}
