#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Instance.h"

namespace v4d::graphics::vulkan {

	class SemaphoreObject {
		COMMON_OBJECT(SemaphoreObject, VkSemaphore, V4DLIB)
		COMMON_OBJECT_MOVEABLE(SemaphoreObject)
		COMMON_OBJECT_COPYABLE(SemaphoreObject)
		
		SemaphoreObject() : obj() {}
		
		Device* device = nullptr;
		
		void Create(Device* device) {
			assert(this->device == nullptr);
			this->device = device;
			
			VkSemaphoreCreateInfo semaphoreInfo {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			Instance::CheckVkResult("Create Semaphore", device->CreateSemaphore(&semaphoreInfo, nullptr, obj));
		}
		void Destroy() {
			assert(device);
			device->DestroySemaphore(obj, nullptr);
			device = nullptr;
		}
	};

	class FenceObject {
		COMMON_OBJECT(FenceObject, VkFence, V4DLIB)
		COMMON_OBJECT_MOVEABLE(FenceObject)
		COMMON_OBJECT_COPYABLE(FenceObject)
		
		bool signaled;
		FenceObject(bool signaled = true) : obj(), signaled(signaled) {}
		
		Device* device = nullptr;
		
		void Create(Device* device) {
			assert(this->device == nullptr);
			this->device = device;
			
			VkFenceCreateInfo fenceInfo = {};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = signaled? VK_FENCE_CREATE_SIGNALED_BIT : 0; // Initialize in the signaled state so that we dont get stuck on the first frame
			Instance::CheckVkResult("Create Fence", device->CreateFence(&fenceInfo, nullptr, obj));
		}
		void Destroy() {
			assert(device);
			device->DestroyFence(obj, nullptr);
			device = nullptr;
		}
	};

	class CommandBufferObject {
		COMMON_OBJECT(CommandBufferObject, VkCommandBuffer, V4DLIB)
		COMMON_OBJECT_MOVEABLE(CommandBufferObject)
		COMMON_OBJECT_COPYABLE(CommandBufferObject)
		
		VkQueueFlags queueFlags = 0;
		uint queueIndex = 0;
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		uint commandPoolIndex = 0;
		
		CommandBufferObject(const VkQueueFlags& queueFlags = 0, uint queueIndex = 0, uint commandPoolIndex = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
		: obj(), queueFlags(queueFlags), queueIndex(uint(queueIndex)), level(level), commandPoolIndex(uint(commandPoolIndex)) {}
		
		Device* device = nullptr;
		
		Queue& GetQueue() {
			assert(device);
			return device->queues[queueFlags][queueIndex];
		}
		
		void Allocate(Device* device) {
			assert(this->device == nullptr);
			this->device = device;
			
			VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandBufferCount = 1;
				allocInfo.level = level;
				allocInfo.commandPool = device->queues[queueFlags][queueIndex].commandPools[commandPoolIndex];
			Instance::CheckVkResult("Allocate CommandBuffer", device->AllocateCommandBuffers(&allocInfo, obj));
		}
		void Free() {
			assert(device);
			device->FreeCommandBuffers(device->queues[queueFlags][queueIndex].commandPools[commandPoolIndex], 1, obj);
			device = nullptr;
		}
	};

}
