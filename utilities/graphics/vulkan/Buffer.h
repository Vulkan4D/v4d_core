/*
 * Vulkan Buffer abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This file contains the Buffer class, the StagedBuffer class and the DeviceLocalBufferPool template
 */
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
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		// Allocated handles
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		
		// Mapped data (this is mapped to when calling MapMemory)
		void* data = nullptr;
		
		// Data pointers to get copied into buffer
		std::vector<BufferSrcDataPtr> srcDataPointers {};
		
		Buffer(VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VkDeviceSize size = 0, bool alignedUniformSize = false);
		
		void ExtendSize(VkDeviceSize);
		
		void AddSrcDataPtr(void* srcDataPtr, size_t size);
		template<class T>
		void AddSrcDataPtr(std::vector<T>* vector) {
			AddSrcDataPtr(vector->data(), vector->size() * sizeof(T));
		}
		template<class T, int S>
		void AddSrcDataPtr(std::array<T, S>* array) {
			AddSrcDataPtr(array->data(), S * sizeof(T));
		}
		
		void ResetSrcData();
		
		void Allocate(Device* device, VkMemoryPropertyFlags properties, bool copySrcData = true, const std::vector<uint32_t>& queueFamilies = {});
		void Free(Device* device);

		void AllocateFromStaging(Device* device, VkCommandBuffer commandBuffer, Buffer& stagingBuffer, VkDeviceSize size = 0, VkDeviceSize offset = 0);

		void MapMemory(Device* device, VkDeviceSize offset = 0, VkDeviceSize size = 0, VkMemoryMapFlags flags = 0);
		void UnmapMemory(Device* device);
		
		void CopySrcData(Device* device, size_t maxCopySize = 0);
		
		void WriteToMappedData(void* inputData, size_t copySize = 0, size_t deviceMemoryOffset = 0);
		void ReadFromMappedData(void* outputData, size_t copySize = 0, size_t deviceMemoryOffset = 0);
		
		// copy from one buffer to another
		static void Copy(Device* device, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
		static void Copy(Device* device, VkCommandBuffer commandBuffer, Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size = 0, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

	};
	
	struct V4DLIB StagedBuffer {
		Buffer stagingBuffer;
		Buffer deviceLocalBuffer;
		
		StagedBuffer(VkBufferUsageFlags usage, VkDeviceSize size = 0, bool alignedUniformSize = false);
		
		void ExtendSize(VkDeviceSize);
		
		void AddSrcDataPtr(void* srcDataPtr, size_t size);
		void ResetSrcData();
		
		template<class T>
		void AddSrcDataPtr(std::vector<T>* vector) {
			AddSrcDataPtr(vector->data(), vector->size() * sizeof(T));
		}
		
		template<class T, int S>
		void AddSrcDataPtr(std::array<T, S>* array) {
			AddSrcDataPtr(array->data(), S * sizeof(T));
		}
		
		void Allocate(Device* device, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, const std::vector<uint32_t>& queueFamilies = {});
		void Free(Device* device);
		
		void Update(Device* device, VkCommandBuffer commandBuffer, size_t maxCopySize = 0);
	};
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DeviceLocalBufferPool template
	
	struct BufferPoolAllocation {
		int bufferIndex;
		int allocationIndex;
		int bufferOffset;
		size_t size;
	};
	
	template<VkBufferUsageFlags usage, size_t DataSize, int NbSubAllocPerBuffer, bool Synchronized = false>
	class DeviceLocalBufferPool {
	private:
		
		struct MultiBuffer {
			Buffer buffer {VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, DataSize * NbSubAllocPerBuffer};
			std::array<bool, NbSubAllocPerBuffer> allocations {false};
			int firstFreeAllocation = 0;
			bool freeOnNextGarbageCollection = false;
			
			int GetAllocationCount() const {
				int count = 0;
				for (bool alloc : allocations) {
					if (alloc) count++;
				}
				return count;
			}
		};
		
		std::vector<MultiBuffer*> buffers {};
		Buffer stagingBuffer {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, DataSize};
		bool stagingBufferAllocated = false;
		int firstFreeBuffer = 0;
		mutable std::mutex sync;
		
		void AllocateStagingBuffer(Device* device) {
			if (!stagingBufferAllocated) {
				stagingBufferAllocated = true;
				stagingBuffer.Allocate(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);
				stagingBuffer.MapMemory(device);
			}
		}
		
		void FreeStagingBuffer(Device* device) {
			if (stagingBufferAllocated) {
				stagingBufferAllocated = false;
				stagingBuffer.UnmapMemory(device);
				stagingBuffer.Free(device);
			}
		}
		
	public:
	
		Buffer* GetBuffer(const int bufferIndex) {
			Buffer* buffer = nullptr;
			if constexpr (Synchronized) sync.lock();
			if (bufferIndex < buffers.size() && buffers[bufferIndex]) 
				buffer = &buffers[bufferIndex]->buffer;
			if constexpr (Synchronized) sync.unlock();
			return buffer;
		}
		
		Buffer* GetBuffer(const BufferPoolAllocation& allocation) {
			return GetBuffer(allocation.bufferIndex);
		}
		
		Buffer* operator[](const int bufferIndex) {
			return GetBuffer(bufferIndex);
		}
		
		Buffer* operator[](const BufferPoolAllocation& allocation) {
			return GetBuffer(allocation.bufferIndex);
		}
		
		int Count() const {
			if constexpr (Synchronized) sync.lock();
			int count = buffers.size() - std::count(buffers.begin(), buffers.end(), nullptr);
			if constexpr (Synchronized) sync.unlock();
			return count;
		}
		
		BufferPoolAllocation Allocate(Device* device, Queue* queue, void* data) {
			if constexpr (Synchronized) sync.lock();
			
			int bufferIndex = firstFreeBuffer;
			int allocationIndex = 0;
			
			// Find first free buffer and allocation
			while (bufferIndex < buffers.size()) {
				auto* multiBuffer = buffers[bufferIndex];
				if (multiBuffer) {
					allocationIndex = multiBuffer->firstFreeAllocation;
					while (allocationIndex < multiBuffer->allocations.size()) {
						if (!multiBuffer->allocations[allocationIndex]) {
							goto CopyData;
						}
						++allocationIndex;
					}
				}
				++bufferIndex;
				allocationIndex = 0;
			}
			
			// Reuse a deleted buffer if possible
			bufferIndex = 0;
			while (bufferIndex < buffers.size()) {
				if (!buffers[bufferIndex]) {
					buffers[bufferIndex] = new MultiBuffer;
					goto Allocate;
				}
				++bufferIndex;
			}
			
			// Empty buffers or no space left in any existing buffers or no deleted buffers, then Add a new buffer
			buffers.emplace_back(new MultiBuffer);
			
			Allocate:
				buffers[bufferIndex]->buffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
				AllocateStagingBuffer(device);
			
			CopyData:
				auto* multiBuffer = buffers[bufferIndex];
				stagingBuffer.WriteToMappedData(data);
				auto cmdBuffer = device->BeginSingleTimeCommands(*queue);
					Buffer::Copy(device, cmdBuffer, stagingBuffer.buffer, multiBuffer->buffer.buffer, DataSize, 0, allocationIndex * DataSize);
				device->EndSingleTimeCommands(*queue, cmdBuffer);
		
			// Activate current allocation
			multiBuffer->allocations[allocationIndex] = true;
			multiBuffer->freeOnNextGarbageCollection = false;
			
			// Cache next free buffer/allocation indices for next allocation calls to be faster
			multiBuffer->firstFreeAllocation = allocationIndex+1;
			if (multiBuffer->firstFreeAllocation < NbSubAllocPerBuffer) {
				firstFreeBuffer = bufferIndex;
			} else {
				firstFreeBuffer = bufferIndex+1;
			}
			
			if constexpr (Synchronized) sync.unlock();
			
			// Return allocation information
			return {bufferIndex, allocationIndex, (int)(allocationIndex * DataSize), DataSize};
		}
		
		void Free(BufferPoolAllocation& allocation) {
			if constexpr (Synchronized) sync.lock();
				if (allocation.bufferIndex < buffers.size() && buffers[allocation.bufferIndex]) {
					buffers[allocation.bufferIndex]->allocations[allocation.allocationIndex] = false;
					buffers[allocation.bufferIndex]->firstFreeAllocation = std::min(buffers[allocation.bufferIndex]->firstFreeAllocation, allocation.allocationIndex);
				}
				firstFreeBuffer = std::min(firstFreeBuffer, allocation.bufferIndex);
			if constexpr (Synchronized) sync.unlock();
		}
		
		void FreePool(Device* device) {
			if constexpr (Synchronized) sync.lock();
				FreeStagingBuffer(device);
				for (auto* multiBuffer : buffers) {
					if (multiBuffer) {
						multiBuffer->buffer.Free(device);
						delete multiBuffer;
					}
				}
				buffers.clear();
				firstFreeBuffer = 0;
			if constexpr (Synchronized) sync.unlock();
		}
		
		void CollectGarbage(Device* device) {
			if constexpr (Synchronized) sync.lock();
			
				firstFreeBuffer = 0;
			
				for (int bufferIndex = buffers.size()-1; bufferIndex >= 0; ) {
					auto* multiBuffer = buffers[bufferIndex];
					if (!multiBuffer) goto Next;
					
					if (multiBuffer->freeOnNextGarbageCollection) {
						multiBuffer->buffer.Free(device);
						delete multiBuffer;
						buffers[bufferIndex] = nullptr;
					} else {
						for (auto alloc : multiBuffer->allocations) 
							if (alloc) goto Next;
						multiBuffer->freeOnNextGarbageCollection = true;
					}
					
					Next:
						bufferIndex--;
				}
				
			if constexpr (Synchronized) sync.unlock();
		}
		
	};

}
