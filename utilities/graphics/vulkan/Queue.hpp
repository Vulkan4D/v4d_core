#pragma once

#include <v4d.h>
#include <vector>
#include <string>
#include "utilities/graphics/vulkan/Loader.h"

namespace v4d::graphics::vulkan {
	
	struct V4DLIB Queue {
		uint32_t familyIndex = 0;
		VkQueue handle = VK_NULL_HANDLE;
		std::array<VkCommandPool, V4D_COMMAND_POOLS_PER_QUEUE> commandPools {};
	};
	
	struct V4DLIB DeviceQueueInfo {
		VkQueueFlags flags;
		uint count;
		std::vector<float> priorities;
		VkSurfaceKHR* surface;
		DeviceQueueInfo(
			VkQueueFlags flags,
			uint count = 1,
			std::vector<float> priorities = {1.0f},
			VkSurfaceKHR* surface = nullptr
		) : 
			flags(flags),
			count(count),
			priorities(priorities),
			surface(surface)
		{}
		
		int queueFamilyIndex = -1;
		int indexOffset = 0;
		int createInfoIndex = -1;
	};

}
