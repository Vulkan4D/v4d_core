#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {
	
	struct V4DLIB Queue {
		uint32_t familyIndex = 0;
		VkQueue handle = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
	};
	
	struct V4DLIB DeviceQueueInfo {
		std::string name;
		VkDeviceQueueCreateFlags flags;
		uint count;
		std::vector<float> priorities;
		VkSurfaceKHR* surface;
		DeviceQueueInfo(
			std::string name,
			VkDeviceQueueCreateFlags flags,
			uint count = 1,
			std::vector<float> priorities = {1.0f},
			VkSurfaceKHR* surface = nullptr
		) : 
			name(name),
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
