#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics::vulkan {
	struct V4DLIB MemoryPool {
		VmaPool handle;
		Device* device;
		MemoryPool(Device*, VkBufferUsageFlags, VmaMemoryUsage, VmaPoolCreateInfo);
		~MemoryPool();
		operator VmaPool() const {return handle;}
	};
}
