#pragma once

#include <v4d.h>

namespace v4d::graphics::vulkan {

	template<class T, size_t COUNT = 1, size_t NB_FRAMES = Renderer::NB_FRAMES_IN_FLIGHT, size_t SIZE = sizeof(T) * COUNT>
	class StagingBuffer {
		VkBuffer deviceLocalBuffer = VK_NULL_HANDLE;
		const VkDeviceSize size = SIZE;
		Device* device = nullptr;
		VkBufferUsageFlags usage;
		std::array<VkBuffer, NB_FRAMES> hostBuffers {};
		std::array<MemoryAllocation, NB_FRAMES> hostBufferAllocations {};
		std::array<void*, NB_FRAMES> hostBufferMappedPointers {};
		MemoryAllocation deviceLocalBufferAllocation = VK_NULL_HANDLE;
		size_t currentFrameIndex = 0;
	public:
		StagingBuffer(VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) : usage(usage) {}
		~StagingBuffer() {
			Free();
		}
		operator BufferDescriptor*() {
			return (BufferDescriptor*)this;
		}
		void Allocate(Device* device) {
			this->device = device;
			// Host buffer
			{VkBufferCreateInfo createInfo {};
				createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				createInfo.size = SIZE;
				createInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				for (int i = 0; i < NB_FRAMES; ++i) {
					device->CreateAndAllocateBuffer(createInfo, MemoryUsage::MEMORY_USAGE_CPU_ONLY, hostBuffers[i], &hostBufferAllocations[i]);
					device->MapMemoryAllocation(hostBufferAllocations[i], &hostBufferMappedPointers[i]);
				}
			}
			// Device buffer
			{VkBufferCreateInfo createInfo {};
				createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				createInfo.size = SIZE;
				createInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				device->CreateAndAllocateBuffer(createInfo, MemoryUsage::MEMORY_USAGE_GPU_ONLY, deviceLocalBuffer, &deviceLocalBufferAllocation);
			}
		}
		void Free() {
			if (device) {
				for (int i = 0; i < NB_FRAMES; ++i) {
					if (hostBufferMappedPointers[i]) {
						device->UnmapMemoryAllocation(hostBufferAllocations[i]);
						hostBufferMappedPointers[i] = nullptr;
					}
					if (hostBuffers[i] && hostBufferAllocations[i]) {
						device->FreeAndDestroyBuffer(hostBuffers[i], hostBufferAllocations[i]);
					}
				}
				if (deviceLocalBuffer && deviceLocalBufferAllocation) {
					device->FreeAndDestroyBuffer(deviceLocalBuffer, deviceLocalBufferAllocation);
				}
				device = nullptr;
			}
		}
		VkBuffer& GetDeviceLocalBuffer() {return deviceLocalBuffer;};
		void SetCurrentFrame(size_t frameIndex) {
			currentFrameIndex = frameIndex % NB_FRAMES;
		}
		T& operator[](size_t index) {
			if (index >= COUNT)
				throw std::runtime_error("Index out of bounds");
			if (!hostBufferMappedPointers[currentFrameIndex]) 
				throw std::runtime_error("Buffer pointers not mapped");
			return ((T*)hostBufferMappedPointers[currentFrameIndex])[index];
		}
		template<typename _T>
		const _T& operator=(const _T& data) {
			if constexpr(sizeof(_T) <= SIZE) {
				memcpy(hostBufferMappedPointers[currentFrameIndex], &data, sizeof(_T));
			}
			return data;
		}
		operator T () const {
			T data;
			memcpy(&data, hostBufferMappedPointers[currentFrameIndex], sizeof(T));
			return data;
		}
		template<typename _T>
		void Init(const _T& data) {
			if constexpr(sizeof(_T) <= SIZE) {
				for (int i = 0; i < NB_FRAMES; ++i) {
					memcpy(hostBufferMappedPointers[i], &data, sizeof(_T));
				}
			}
		}
		T* operator->() {
			return (T*)hostBufferMappedPointers[currentFrameIndex];
		}
		T& operator*() {
			return *(T*)hostBufferMappedPointers[currentFrameIndex];
		}
		void Push(VkCommandBuffer cmbBuffer, uint32_t count = COUNT, VkDeviceSize offset = 0) {
			if (count == 0) return;
			assert(count <= COUNT);
			if (device && deviceLocalBuffer && hostBuffers[currentFrameIndex]) {
				VkBufferCopy region = {};{
					region.srcOffset = offset;
					region.dstOffset = offset;
					region.size = count * sizeof(T);
				}
				device->CmdCopyBuffer(cmbBuffer, hostBuffers[currentFrameIndex], deviceLocalBuffer, 1, &region);
			}
		}
		void Pull(VkCommandBuffer cmbBuffer, uint32_t count = COUNT, VkDeviceSize offset = 0) {
			if (count == 0) return;
			assert(count <= COUNT);
			if (device && deviceLocalBuffer && hostBuffers[currentFrameIndex]) {
				VkBufferCopy region = {};{
					region.srcOffset = offset;
					region.dstOffset = offset;
					region.size = count * sizeof(T);
				}
				device->CmdCopyBuffer(cmbBuffer, deviceLocalBuffer, hostBuffers[currentFrameIndex], 1, &region);
			}
		}
	};
}
