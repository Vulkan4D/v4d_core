#pragma once
#include <v4d.h>

#define XVK_INTERFACE_RAW_FUNCTIONS_ACCESSIBILITY private
#include <xvk.hpp>

#define V4D_ENGINE_NAME "Vulkan4D"
#define V4D_ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)
#define VULKAN_API_VERSION VK_API_VERSION_1_1

namespace v4d::graphics::vulkan {
	
	class V4DLIB Loader : public xvk::Loader {
	public:
		std::vector<const char*> requiredInstanceExtensions {
			#ifdef _DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			#endif
		};
		std::vector<const char*> requiredInstanceLayers {
			#ifdef _DEBUG
			"VK_LAYER_LUNARG_standard_validation",
			#endif
		};
		
		void CheckExtensions(bool logging = false);
		void CheckLayers(bool logging = false);
		void CheckVulkanVersion();

	};
	
	//////////////////////////////////////////////////////
	
	struct V4DLIB VertexInputAttributeDescription {
		uint32_t location;
		uint32_t offset;
		VkFormat format;
	};

	struct V4DLIB Queue {
		uint32_t familyIndex;
		VkQueue handle = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
	};

}
