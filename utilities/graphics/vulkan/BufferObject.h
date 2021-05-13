#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics::vulkan {

using FrameBufferedBuffer = std::array<VkBuffer, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES>;

class V4DLIB BufferObject {
	COMMON_OBJECT(BufferObject, VkBuffer, V4DLIB)
	VkDeviceSize size;
	MemoryUsage memoryUsage;
	VkBufferUsageFlags bufferUsage;
	
	Device* device = nullptr;
	MemoryAllocation allocation = VK_NULL_HANDLE;
	
	BufferObject(VkDeviceSize size, MemoryUsage memoryUsage = MEMORY_USAGE_CPU_TO_GPU, VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : obj(), size(size), memoryUsage(memoryUsage), bufferUsage(bufferUsage) {}
	
	virtual void Allocate(Device* device);
	virtual void Free();
	
	virtual ~BufferObject() {
		assert(!device && !allocation);
	}
	
	operator FrameBufferedBuffer() const {
		FrameBufferedBuffer buffers;
		buffers.fill(obj);
		return buffers;
	}
};

template<typename T, int COUNT = 1>
class MappedBufferObject : public BufferObject {
protected:
	T* data;
	
public:
	MappedBufferObject(MemoryUsage memoryUsage = MEMORY_USAGE_CPU_TO_GPU, VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : BufferObject(sizeof(T) * COUNT, memoryUsage, bufferUsage), data(nullptr) {}
	
	virtual void Allocate(Device* device) override {
		BufferObject::Allocate(device);
		device->MapMemoryAllocation(allocation, &data, 0, size);
	}
	
	virtual void Free() override {
		assert(device);
		if (data) {
			device->UnmapMemoryAllocation(allocation);
			data = nullptr;
		}
		BufferObject::Free();
	}
	
	T& operator[](size_t index) {
		assert(index < COUNT);
		return data[index];
	}
	T* operator->() {return data;}
	operator T&() {return *data;}
	
};

template<typename T, int COUNT = 1>
class StagingBuffer {
protected:
	MappedBufferObject<T> hostBuffer;
	BufferObject deviceBuffer;
	
public:
	StagingBuffer(VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : hostBuffer(COUNT, MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
	 , deviceBuffer(sizeof(T) * COUNT, MEMORY_USAGE_GPU_ONLY, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
	{}
	
	T& operator[](size_t index) {
		return hostBuffer[index];
	}
	T* operator->() {return hostBuffer.operator->();}
	operator T&() {return hostBuffer;}
	operator VkBuffer&() {return hostBuffer.obj;}
	
	operator FrameBufferedBuffer() const {
		FrameBufferedBuffer buffers;
		buffers.fill(deviceBuffer.obj);
		return buffers;
	}
	
	void Push(VkCommandBuffer cmdBuffer, uint32_t count = COUNT, VkDeviceSize offset = 0) {
		if (count == 0) return;
		assert(hostBuffer.device);
		assert(count <= COUNT);
		VkBufferCopy region = {};{
			region.srcOffset = offset;
			region.dstOffset = offset;
			region.size = count * sizeof(T);
		}
		hostBuffer.device->CmdCopyBuffer(cmdBuffer, hostBuffer, deviceBuffer, 1, &region);
	}
	
	void Pull(VkCommandBuffer cmdBuffer, uint32_t count = COUNT, VkDeviceSize offset = 0) {
		if (count == 0) return;
		assert(hostBuffer.device);
		assert(count <= COUNT);
		VkBufferCopy region = {};{
			region.srcOffset = offset;
			region.dstOffset = offset;
			region.size = count * sizeof(T);
		}
		hostBuffer.device->CmdCopyBuffer(cmdBuffer, deviceBuffer, hostBuffer, 1, &region);
	}
	
};

}
