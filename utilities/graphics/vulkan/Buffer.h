#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics::vulkan {

struct V4DLIB Buffer {
	VkBuffer handle;
	
	MemoryUsage memoryUsage;
	VkBufferUsageFlags bufferUsage;
	VkDeviceSize size;
	uint32_t alignment;
	
	Device* device = nullptr;
	MemoryAllocation allocation = VK_NULL_HANDLE;
	VkDeviceOrHostAddressConstKHR address {};
	VkDeviceSize alignedOffset = 0;
	
	std::recursive_mutex allocationMutex;
	
	VmaPool pool = VK_NULL_HANDLE;
	
	auto Lock() {
		return std::unique_lock<std::recursive_mutex>(allocationMutex);
	}
	
	Buffer(MemoryUsage memoryUsage, VkBufferUsageFlags bufferUsage, VkDeviceSize size, uint32_t alignment = 0)
	 : memoryUsage(memoryUsage), bufferUsage(bufferUsage), size(size), alignment(alignment) {}
	
	Buffer(VmaPool pool, VkBufferUsageFlags bufferUsage, VkDeviceSize size, uint32_t alignment = 0)
	 : memoryUsage(MEMORY_USAGE_UNKNOWN), bufferUsage(bufferUsage), size(size), alignment(alignment), pool(pool) {}
	
	Buffer()
	 : memoryUsage(MEMORY_USAGE_UNKNOWN), bufferUsage(0), size(0), alignment(0) {}
	
	DELETE_COPY_MOVE_CONSTRUCTORS(Buffer)
	
	virtual void Allocate(Device* device);
	virtual void Free();
	virtual void Resize(size_t newSize, bool allowReallocation = false) {
		assert(device == nullptr || allowReallocation);
		size = newSize;
		if (device != nullptr && allowReallocation) {
			auto dev = device;
			Free();
			Allocate(dev);
		}
	}
	
	virtual ~Buffer();
	
	// NOT Thread-Safe
	void Swap(Buffer& other) {
		assert(memoryUsage == other.memoryUsage);
		assert(bufferUsage == other.bufferUsage);
		assert(size == other.size);
		assert(device == other.device);
		assert(alignment == other.alignment);
		std::swap(handle, other.handle);
		std::swap(address, other.address);
		std::swap(allocation, other.allocation);
		std::swap(alignedOffset, other.alignedOffset);
	}
	
	void ZeroInitialize(VkCommandBuffer cmdBuffer) {
		device->CmdFillBuffer(cmdBuffer, handle, 0, VK_WHOLE_SIZE, 0);
	}
	
	operator VkDeviceOrHostAddressConstKHR() const {return address;}
	operator VkDeviceAddress() const {return address.deviceAddress;}
	
	operator const VkBuffer&() const {return handle;}
	operator VkBuffer&() {return handle;}
	operator const VkBuffer* const() const {return &handle;}
	operator VkBuffer*() {return &handle;}
	
	// for deferred deallocation
	void Touch() {
		touchedInFrameIndex = currentFrameIndex;
	}
private: friend class Device;
	uint64_t touchedInFrameIndex = 0;
	static uint64_t currentFrameIndex;
};

template<typename T>
class MappedBuffer : public Buffer {
protected:
	T* data;
	std::vector<T> temporaryData {};
public:
	MappedBuffer(MemoryUsage memoryUsage, VkBufferUsageFlags bufferUsage, uint32_t count = 1)
	 : Buffer(
			memoryUsage,
			bufferUsage
				| VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT // This forces the memory alignment to 16 bytes for all mapped buffers (fixes lots of issues with -O3 compiler optimisations)... Maybe find a better solution in the future...
				,
			sizeof(T) * count
		), data(nullptr) {}
	
	MappedBuffer(VmaPool pool, VkBufferUsageFlags bufferUsage, uint32_t count = 1)
	 : Buffer(pool, bufferUsage, sizeof(T) * count), data(nullptr) {}
	
	MappedBuffer() {}
	
	DELETE_COPY_MOVE_CONSTRUCTORS(MappedBuffer)
	
	static inline const size_t TypeSize = sizeof(T);
	
	size_t Count() const {
		return size / sizeof(T);
	}
	
	void UseTemporaryData() {
		assert(!data);
		assert(size);
		if (temporaryData.size() != Count()) temporaryData.resize(Count());
		data = temporaryData.data();
	}
	
	virtual void Allocate(Device* device) override {
		std::lock_guard lock(allocationMutex);
		if (size > 0 && this->device == nullptr) {
			if (device) {
				Buffer::Allocate(device);
				device->MapMemoryAllocation(allocation, (void**)&data, 0, size);
				if (temporaryData.size() == Count()) {
					Fill(temporaryData);
					temporaryData = {};
				}
			} else {
				UseTemporaryData();
			}
		}
	}
	
	void ZeroInitialize() {
		ZeroInitialize(size);
	}
	
	void ZeroInitialize(size_t size, size_t offset = 0) {
		if (!data) UseTemporaryData();
		memset(data + offset, 0, size);
	}
	
	void Fill(const void* src, size_t size, size_t dstOffset = 0) {
		if (!data) UseTemporaryData();
		assert(src);
		assert(size + dstOffset <= this->size);
		memcpy(data + dstOffset, src, size);
	}
	
	void Fill(const std::vector<T>& src) {
		if (!data) UseTemporaryData();
		assert(src.size() <= Count());
		memcpy(data, src.data(), src.size()*sizeof(T));
	}
	
	virtual void Free() override {
		std::lock_guard lock(allocationMutex);
		if (device) {
			if (data) {
				data = nullptr;
				device->UnmapMemoryAllocation(allocation);
			}
			Buffer::Free();
		} else {
			data = nullptr;
			temporaryData = std::vector<T>(); // Forces deallocation
		}
	}
	
	virtual ~MappedBuffer() {
		Free();
	}
	
	virtual void Resize(size_t newCount, bool allowReallocation = false) override {
		assert(device == nullptr || allowReallocation);
		if (size > 0 && temporaryData.size() == Count()) {
			temporaryData.resize(newCount);
		}
		Buffer::Resize(newCount * sizeof(T), allowReallocation);
	}
	
	T& operator[](size_t index) {
		assert(data);
		assert(index * sizeof(T) < size);
		return data[index];
	}
	T* Data() {
		assert(data);
		return data;
	}
	T* operator->() {
		return Data();
	}
	T& operator*() {
		return *this->Data();
	}
	template<typename OTHER>
	T& operator=(const OTHER& other) {
		assert(data);
		return *data = other;
	}
	operator VkDeviceOrHostAddressConstKHR() const {return address;}
	operator VkDeviceAddress() const {return address.deviceAddress;}
};

template<typename T>
class StagingBuffer {
protected:
	MappedBuffer<T> hostBuffer;
	Buffer deviceBuffer;
public:
	StagingBuffer(VkBufferUsageFlags bufferUsage, uint32_t count = 1, uint32_t alignment = 0)
	 : hostBuffer(MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, count)
	 , deviceBuffer(MEMORY_USAGE_GPU_ONLY, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(T) * count, alignment)
	{}
	StagingBuffer(VmaPool hostPool, VmaPool devicePool, VkBufferUsageFlags bufferUsage, uint32_t count = 1, uint32_t alignment = 0)
	 : hostBuffer(hostPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, count)
	 , deviceBuffer(devicePool, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(T) * count, alignment)
	{
		if (!hostPool) hostBuffer.memoryUsage = MEMORY_USAGE_CPU_ONLY;
		if (!devicePool) deviceBuffer.memoryUsage = MEMORY_USAGE_GPU_ONLY;
	}
	StagingBuffer() {}
	
	DELETE_COPY_MOVE_CONSTRUCTORS(StagingBuffer)
	
	static inline const size_t TypeSize = sizeof(T);
	
	bool dirty = false; // For your own usage
	
	size_t Count() const {
		return hostBuffer.Count();
	}
	
	auto Lock() {
		return hostBuffer.Lock();
	}
	
	void Allocate(Device* device) {
		std::lock_guard lock(hostBuffer.allocationMutex);
		hostBuffer.Allocate(device);
		if (device) deviceBuffer.Allocate(device);
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
		std::lock_guard lock(hostBuffer.allocationMutex);
		hostBuffer.Free();
		deviceBuffer.Free();
	}
	
	void Resize(size_t newCount, bool allowReallocation = false) {
		std::lock_guard lock(hostBuffer.allocationMutex);
		assert(hostBuffer.device == nullptr || allowReallocation);
		hostBuffer.Resize(newCount, allowReallocation);
		deviceBuffer.Resize(newCount * sizeof(T), allowReallocation);
	}
	
	T& operator[](size_t index) {
		return hostBuffer[index];
	}
	T* Data() {return hostBuffer.Data();}
	T* operator->() {return hostBuffer.Data();}
	operator T&() {return hostBuffer;}
	operator VkBuffer&() {return deviceBuffer;}
	operator Buffer&() {return deviceBuffer;}
	operator const Buffer&() const {return deviceBuffer;}
	explicit operator bool() {return !!VkBuffer(hostBuffer);}
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
				region.dstOffset = offset + deviceBuffer.alignedOffset;
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
				region.srcOffset = offset + deviceBuffer.alignedOffset;
				region.dstOffset = offset;
				region.size = count > 0? (uint32_t(count) * sizeof(T)) : hostBuffer.size;
			}
			hostBuffer.device->CmdCopyBuffer(cmdBuffer, deviceBuffer, hostBuffer, 1, &region);
		}
	}
	
	void Touch() {
		hostBuffer.Touch();
		deviceBuffer.Touch();
	}
};

}
