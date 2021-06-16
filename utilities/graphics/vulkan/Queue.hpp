#pragma once

#include <v4d.h>
#include <vector>
#include <string>
#include "utilities/graphics/vulkan/Loader.h"

namespace v4d::graphics::vulkan {
	
	struct V4DLIB Queue {
		VkQueueFlags flags;
		std::vector<VkCommandPool> commandPools;
		VkSurfaceKHR* surface;
		float priority;
		
		Queue(
			VkQueueFlags flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
			size_t nbCommandPools = 1,
			VkSurfaceKHR* surface = nullptr,
			float priority = 1.0f
		) : flags(flags), commandPools(nbCommandPools), surface(surface), priority(priority) {}
		
		uint32_t family = 0;
		uint32_t index = 0;
		VkQueue handle = VK_NULL_HANDLE;
	};
	
}
