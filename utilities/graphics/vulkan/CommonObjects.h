#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Instance.h"

namespace v4d::graphics::vulkan {

	class SemaphoreObject {
		COMMON_OBJECT(SemaphoreObject, VkSemaphore, V4DLIB)
		
		void Create(Device* device) {
			VkSemaphoreCreateInfo semaphoreInfo {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			Instance::CheckVkResult("Create Semaphore", device->CreateSemaphore(&semaphoreInfo, nullptr, obj));
		}
		void Destroy(Device* device) {
			device->DestroySemaphore(obj, nullptr);
		}
	};

	class FenceObject {
		COMMON_OBJECT(FenceObject, VkFence, V4DLIB)
		
		bool signaled = true;
		explicit FenceObject(bool signaled) : obj(), signaled(signaled) {}
		
		void Create(Device* device) {
			VkFenceCreateInfo fenceInfo = {};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = signaled? VK_FENCE_CREATE_SIGNALED_BIT : 0; // Initialize in the signaled state so that we dont get stuck on the first frame
			Instance::CheckVkResult("Create Fence", device->CreateFence(&fenceInfo, nullptr, obj));
		}
		void Destroy(Device* device) {
			device->DestroyFence(obj, nullptr);
		}
	};

	class CommandBufferObject {
		COMMON_OBJECT(CommandBufferObject, VkCommandBuffer, V4DLIB)
		
		VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
		uint queueIndex = 0;
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		CommandBufferObject(VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT, uint queueIndex = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
		: obj(), queueFlags(queueFlags), queueIndex(queueIndex), level(level) {}
		
		void Allocate(Device* device) {
			VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandBufferCount = 1;
				allocInfo.level = level;
				allocInfo.commandPool = device->GetQueue(queueFlags, queueIndex).commandPool;
			Instance::CheckVkResult("Allocate CommandBuffer", device->AllocateCommandBuffers(&allocInfo, obj));
		}
		void Free(Device* device) {
			device->FreeCommandBuffers(device->GetGraphicsQueue().commandPool, 1, obj);
		}
	};

}
