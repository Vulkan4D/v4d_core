/*
 * Vulkan Dynamic Loader
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class extends from xvk's dynamic loader (xvk is another open-source library related to Vulkan4D)
 */
#pragma once

#include <v4d.h>
#include <vector>

#ifndef XVK_INTERFACE_RAW_FUNCTIONS_ACCESSIBILITY
	#define XVK_INTERFACE_RAW_FUNCTIONS_ACCESSIBILITY private
#endif

#ifdef V4D_VULKAN_USE_VMA
	#define XVK_INCLUDE_VMA
#endif

#include <xvk.hpp>

#define V4D_ENGINE_NAME "Vulkan4D"
#define V4D_ENGINE_VERSION VK_MAKE_VERSION(V4D_VERSION_MAJOR, V4D_VERSION_MINOR, V4D_VERSION_PATCH)

namespace v4d::graphics::vulkan {
	
	class V4DLIB Loader : public xvk::Loader {
	public:
		using xvk::Loader::Loader;
		static uint32_t VULKAN_API_VERSION;
		static uint32_t VULKAN_API_VERSION_ALTERNATIVE;
	
		// Required Instance Extensions
		std::vector<const char*> requiredInstanceExtensions {
			#ifdef V4D_VULKAN_USE_VALIDATION_LAYERS
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			#endif
			#ifdef V4D_VULKAN_USE_VMA
				VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			#endif
		};
		
		// Required Instance Layers
		std::vector<const char*> requiredInstanceLayers {
			#ifdef V4D_VULKAN_USE_VALIDATION_LAYERS
				"VK_LAYER_KHRONOS_validation",
			#endif
		};
		
		#ifndef XVK_USE_QT_VULKAN_LOADER
			void CheckExtensions();
			void CheckLayers();
			void CheckVulkanVersion();
		#endif
	};
	
}
