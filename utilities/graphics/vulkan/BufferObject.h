#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics::vulkan {

using FramebufferedBuffer = std::array<VkBuffer, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES>;

class V4DLIB BufferObject {
	COMMON_OBJECT(BufferObject, VkBuffer, V4DLIB)
	MemoryUsage memoryUsage;
	VkBufferUsageFlags bufferUsage;
	VkDeviceSize size;
	
	Device* device = nullptr;
	MemoryAllocation allocation = VK_NULL_HANDLE;
	VkDeviceOrHostAddressConstKHR address {};
	
	BufferObject(MemoryUsage memoryUsage, VkBufferUsageFlags bufferUsage, VkDeviceSize size)
	 : obj(), memoryUsage(memoryUsage), bufferUsage(bufferUsage), size(size) {}
	
	BufferObject()
	 : obj(), memoryUsage(MEMORY_USAGE_UNKNOWN), bufferUsage(0), size(0) {}
	
	virtual void Allocate(Device* device);
	virtual void Free();
	virtual void Resize(size_t newSize) {
		assert(device == nullptr);
		size = newSize;
	}
	
	virtual ~BufferObject();
	
	// operator FramebufferedBuffer() const {
	// 	FramebufferedBuffer buffers;
	// 	buffers.fill(obj); // fills the array with VkBuffer pointers, not data...
	// 	return buffers;
	// }
	
	void Swap(BufferObject& other) {
		assert(memoryUsage == other.memoryUsage);
		assert(bufferUsage == other.bufferUsage);
		assert(size == other.size);
		assert(device == other.device);
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);
		std::swap(obj.obj, other.obj.obj);
		std::swap(address, other.address);
		std::swap(allocation, other.allocation);
	}
	
	operator VkDeviceOrHostAddressConstKHR() const {return address;}
	operator VkDeviceAddress() const {return address.deviceAddress;}
};

template<typename T>
class MappedBufferObject : public BufferObject {
protected:
	T* data;
	std::vector<T> temporaryData {};
public:
	MappedBufferObject(MemoryUsage memoryUsage, VkBufferUsageFlags bufferUsage, uint32_t count = 1)
	 : BufferObject(
			memoryUsage,
			bufferUsage
				| VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT // This forces the memory alignment to 16 bytes for all mapped buffers (fixes lots of issues with -O3 compiler optimisations)... Maybe find a better solution in the future...
				,
			sizeof(T) * count
		), data(nullptr) {}
	
	MappedBufferObject() {}
	
	static inline const size_t TypeSize = sizeof(T);
	
	size_t Count() const {
		return size / sizeof(T);
	}
	
	virtual void Allocate(Device* device) override {
		if (size > 0 && this->device == nullptr) {
			BufferObject::Allocate(device);
			device->MapMemoryAllocation(allocation, (void**)&data, 0, size);
			if (temporaryData.size() == Count()) {
				Fill(temporaryData);
			}
		}
	}
	
	void ZeroInitialize() {
		ZeroInitialize(size);
	}
	
	void ZeroInitialize(size_t size, size_t offset = 0) {
		assert(data);
		memset(data + offset, 0, size);
	}
	
	void Fill(const void* src, size_t size, size_t dstOffset = 0) {
		assert(data);
		assert(src);
		assert(size + dstOffset <= this->size);
		memcpy(data + dstOffset, src, size);
	}
	
	void Fill(const std::vector<T>& src) {
		assert(data);
		assert(src.size() <= Count());
		memcpy(data, src.data(), src.size()*sizeof(T));
	}
	
	virtual void Free() override {
		if (device && data) {
			if (size > 0 && temporaryData.size() == Count()) {
				memcpy(temporaryData.data(), data, size);
			}
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
		if (size > 0 && temporaryData.size() == Count()) {
			temporaryData.resize(newCount);
		}
		BufferObject::Resize(newCount * sizeof(T));
	}
	
	T& operator[](size_t index) {
		assert(index * sizeof(T) < size);
		if (data) {
			return data[index];
		}
		if (temporaryData.size() != Count()) temporaryData.resize(Count());
		return temporaryData[index];
	}
	T* operator->() {
		if (data) return data;
		if (temporaryData.size() != Count()) temporaryData.resize(Count());
		return temporaryData.data();
	}
	operator T&() {
		return *this->operator->();
	}
	explicit operator bool() {return size > 0 && data;}
	template<typename OTHER>
	T& operator=(const OTHER& other) {
		if (data) return data[0] = other;
		if (temporaryData.size() != Count()) temporaryData.resize(Count());
		return temporaryData[0] = other;
	}
	operator VkDeviceOrHostAddressConstKHR() const {return address;}
	operator VkDeviceAddress() const {return address.deviceAddress;}
};

template<typename T>
class StagingBuffer {
protected:
	MappedBufferObject<T> hostBuffer;
	BufferObject deviceBuffer;
public:
	StagingBuffer(VkBufferUsageFlags bufferUsage, uint32_t count = 1)
	 : hostBuffer(MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, count)
	 , deviceBuffer(MEMORY_USAGE_GPU_ONLY, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(T) * count)
	{}
	StagingBuffer() {}
	
	static inline const size_t TypeSize = sizeof(T);
	
	size_t Count() const {
		return hostBuffer.Count();
	}
	
	void Allocate(Device* device) {
		hostBuffer.Allocate(device);
		deviceBuffer.Allocate(device);
	}
	
	void ZeroInitialize() {
		hostBuffer.ZeroInitialize();
	}
	
	void ZeroInitialize(size_t size, size_t offset = 0) {
		hostBuffer.ZeroInitialize(size, offset);
	}
	
	void Fill(const void* src, size_t size, size_t dstOffset = 0) {
		hostBuffer.Fill(src, size, dstOffset);
	}
	
	void Fill(const std::vector<T>& src) {
		hostBuffer.Fill(src);
	}
	
	void Free() {
		hostBuffer.Free();
		deviceBuffer.Free();
	}
	
	void Resize(size_t newCount) {
		assert(hostBuffer.device == nullptr);
		hostBuffer.Resize(newCount);
		deviceBuffer.Resize(newCount * sizeof(T));
	}
	
	T& operator[](size_t index) {
		return hostBuffer[index];
	}
	T* operator->() {return hostBuffer.operator->();}
	operator T&() {return hostBuffer;}
	operator VkBuffer&() {return deviceBuffer;}
	operator BufferObject&() {return deviceBuffer;}
	operator const BufferObject&() const {return deviceBuffer;}
	explicit operator bool() {return bool(hostBuffer);}
	operator VkDeviceOrHostAddressConstKHR() {return deviceBuffer;}
	operator VkDeviceAddress() {return deviceBuffer;}
	
	template<typename OTHER>
	T& operator=(const OTHER& other) {
		return hostBuffer = other;
	}
	
	void Push(VkCommandBuffer cmdBuffer, int32_t count = -1, VkDeviceSize offset = 0) {
		if (count == 0) return;
		if (hostBuffer.size > 0) {
			assert(hostBuffer.device);
			assert(count * int32_t(sizeof(T)) <= int32_t(hostBuffer.size));
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
			assert(count * int32_t(sizeof(T)) <= int32_t(hostBuffer.size));
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
