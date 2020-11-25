#include <v4d.h>

namespace v4d::tests {
	int VulkanInstance() {
		
		v4d::graphics::vulkan::Loader vulkanLoader;
		
		if (!vulkanLoader()) {
			LOG_ERROR("Failed to load vulkan")
			return -1;
		}
		
		try {
			v4d::graphics::vulkan::Instance instance(&vulkanLoader, "Test", VK_MAKE_VERSION(1, 0, 0));
		} catch(...) {
			LOG_ERROR("Failed to create vulkan instance")
			return -2;
		}
		
		return 0;
	}
}
