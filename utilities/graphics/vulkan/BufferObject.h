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
	VkDeviceOrHostAddressConstKHR address {};
	
	BufferObject(VkDeviceSize size, MemoryUsage memoryUsage = MEMORY_USAGE_CPU_TO_GPU, VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : obj(), size(size), memoryUsage(memoryUsage), bufferUsage(bufferUsage) {}
	
	virtual void Allocate(Device* device);
	virtual void Free();
	virtual void Resize(size_t newSize) {
		assert(device == nullptr);
		size = newSize;
	}
	
	virtual ~BufferObject();
	
	operator FrameBufferedBuffer() const {
		FrameBufferedBuffer buffers;
		buffers.fill(obj); // fills the array with VkBuffer pointers, not data...
		return buffers;
	}
	
	operator VkDeviceOrHostAddressConstKHR() {return address;}
	operator VkDeviceAddress() {return address.deviceAddress;}
};

template<typename T>
class MappedBufferObject : public BufferObject {
protected:
	T* data;
	
public:
	MappedBufferObject(uint32_t count = 1, MemoryUsage memoryUsage = MEMORY_USAGE_CPU_TO_GPU, VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : BufferObject(sizeof(T) * count, memoryUsage, bufferUsage), data(nullptr) {}
	
	static inline const size_t TypeSize = sizeof(T);
	
	virtual void Allocate(Device* device) override {
		if (size > 0) {
			BufferObject::Allocate(device);
			device->MapMemoryAllocation(allocation, (void**)&data, 0, size);
		}
	}
	
	virtual void Free() override {
		assert(device);
		if (data) {
			device->UnmapMemoryAllocation(allocation);
			data = nullptr;
		}
		BufferObject::Free();
	}
	
	virtual ~MappedBufferObject() {
		Free();
	}
	
	virtual void Resize(size_t newCount) {
		assert(device == nullptr);
		BufferObject::Resize(newCount * sizeof(T));
	}
	
	T& operator[](size_t index) {
		assert(index * sizeof(T) < size);
		return data[index];
	}
	T* operator->() {assert(data); return data;}
	operator T&() {assert(data); return *data;}
	operator bool() {return size > 0 && data;}
};

template<typename T>
class StagingBuffer {
protected:
	MappedBufferObject<T> hostBuffer;
	BufferObject deviceBuffer;
public:
	StagingBuffer(uint32_t count = 1, VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	 : hostBuffer(count, MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
	 , deviceBuffer(sizeof(T) * count, MEMORY_USAGE_GPU_ONLY, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
	{}
	
	static inline const size_t TypeSize = sizeof(T);
	
	void Resize(size_t newCount) {
		assert(hostBuffer.device == nullptr);
		hostBuffer.Resize(newCount);
		deviceBuffer.Resize(newCount * sizeof(T));
	}
	
	T& operator[](size_t index) {
		assert(index * sizeof(T) < hostBuffer.size);
		return hostBuffer[index];
	}
	T* operator->() {return hostBuffer.operator->();}
	operator T&() {return hostBuffer;}
	operator VkBuffer&() {return deviceBuffer;}
	operator bool() {return hostBuffer;}
	operator VkDeviceOrHostAddressConstKHR() {return deviceBuffer;}
	operator VkDeviceAddress() {return deviceBuffer;}
	
	operator FrameBufferedBuffer() const {
		assert(hostBuffer.size > 0);
		FrameBufferedBuffer buffers;
		buffers.fill(deviceBuffer.obj);
		return buffers;
	}
	
	void Push(VkCommandBuffer cmdBuffer, int32_t count = -1, VkDeviceSize offset = 0) {
		if (count == 0) return;
		if (hostBuffer.size > 0) {
			assert(hostBuffer.device);
			assert(uint32_t(count) * sizeof(T) <= hostBuffer.size);
			VkBufferCopy region = {};{
				region.srcOffset = offset;
				region.dstOffset = offset;
				region.size = count > 0? (uint32_t(count) * sizeof(T)) : hostBuffer.size;
			}
			hostBuffer.device->CmdCopyBuffer(cmdBuffer, hostBuffer, deviceBuffer, 1, &region);
		}
	}
	
	void Pull(VkCommandBuffer cmdBuffer, uint32_t count = -1, VkDeviceSize offset = 0) {
		if (count == 0) return;
		if (hostBuffer.size > 0) {
			assert(hostBuffer.device);
			assert(uint32_t(count) * sizeof(T) <= hostBuffer.size);
			VkBufferCopy region = {};{
				region.srcOffset = offset;
				region.dstOffset = offset;
				region.size = count > 0? (uint32_t(count) * sizeof(T)) : hostBuffer.size;
			}
			hostBuffer.device->CmdCopyBuffer(cmdBuffer, deviceBuffer, hostBuffer, 1, &region);
		}
	}
};

}
