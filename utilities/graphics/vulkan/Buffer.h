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
		void ResetSrcData();
		
		template<class T>
		void AddSrcDataPtr(std::vector<T>* vector) {
			AddSrcDataPtr(vector->data(), vector->size() * sizeof(T));
		}
		
		template<class T, int S>
		void AddSrcDataPtr(std::array<T, S>* array) {
			AddSrcDataPtr(array->data(), S * sizeof(T));
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
	
	struct BufferPoolAllocation {
		int bufferIndex;
		int allocationIndex;
		int bufferOffset;
	};
	
	template<VkBufferUsageFlags usage, size_t size, int n = 1>
	class DeviceLocalBufferPool {
		struct MultiBuffer {
			Buffer buffer {VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, size * n};
			std::array<bool, n> allocations {false};
		};
		std::vector<MultiBuffer> buffers {};
		Buffer stagingBuffer {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size};
		bool stagingBufferAllocated = false;
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

		Buffer* operator[](int bufferIndex) {
			if (bufferIndex > buffers.size() - 1) return nullptr;
			return &buffers[bufferIndex].buffer;
		}
		
		BufferPoolAllocation Allocate(Device* device, Queue* queue, void* data) {
			int bufferIndex = 0;
			int allocationIndex = 0;
			for (auto& multiBuffer : buffers) {
				for (bool& offsetUsed : multiBuffer.allocations) {
					if (!offsetUsed) {
						offsetUsed = true;
						goto CopyData;
					}
					++allocationIndex;
				}
				++bufferIndex;
				allocationIndex = 0;
			}
			goto AllocateNewBuffer;
			
			AllocateNewBuffer: {
				auto& multiBuffer = buffers.emplace_back();
				auto cmdBuffer = device->BeginSingleTimeCommands(*queue);
				AllocateStagingBuffer(device);
				stagingBuffer.WriteToMappedData(device, data);
				multiBuffer.buffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
				Buffer::Copy(device, cmdBuffer, stagingBuffer.buffer, multiBuffer.buffer.buffer, size, 0, allocationIndex * size);
				device->EndSingleTimeCommands(*queue, cmdBuffer);
				multiBuffer.allocations[allocationIndex] = true;
				goto Return;
			}
			
			CopyData: {
				auto& multiBuffer = buffers[bufferIndex];
				auto cmdBuffer = device->BeginSingleTimeCommands(*queue);
				AllocateStagingBuffer(device);
				stagingBuffer.WriteToMappedData(device, data);
				Buffer::Copy(device, cmdBuffer, stagingBuffer.buffer, multiBuffer.buffer.buffer, size, 0, allocationIndex * size);
				device->EndSingleTimeCommands(*queue, cmdBuffer);
			}
			
			Return: return {bufferIndex, allocationIndex, (int)(allocationIndex * size)};
		}
		
		void Free(BufferPoolAllocation allocation) {
			buffers[allocation.bufferIndex].allocations[allocation.allocationIndex] = false;
		}
		
		void FreePool(Device* device) {
			FreeStagingBuffer(device);
			for (auto& bufferAllocation : buffers) {
				bufferAllocation.buffer.Free(device);
			}
			buffers.clear();
		}
		
	};

}
