/*
 * Vulkan Instance abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class extends from xvk's Vulkan Instance (xvk is another open-source library related to Vulkan4D)
 */
#pragma once

#include <v4d.h>
#include <vector>
#include <functional>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/PhysicalDevice.h"

namespace v4d::graphics::vulkan {

	class V4DLIB Instance : public xvk::Interface::InstanceInterface {
	protected:
		vulkan::Loader* vulkanLoader;

	private:
		std::vector<PhysicalDevice*> availablePhysicalDevices;

		void LoadAvailablePhysicalDevices();

	public:
		#ifdef XVK_USE_QT_VULKAN_LOADER
			Instance(vulkan::Loader* loader);
		#else
			Instance(vulkan::Loader* loader, const char* applicationName, uint applicationVersion);
		#endif

		virtual ~Instance();

		VkInstance GetHandle() const;

		PhysicalDevice* SelectSuitablePhysicalDevice(const std::function<void(int&, PhysicalDevice*)>& suitabilityFunc);

		static std::string GetVkResultText(const VkResult& res) {
			switch (res) {
				case VK_SUCCESS: return "SUCCESS";
				case VK_NOT_READY: return "NOT_READY";
				case VK_TIMEOUT: return "TIMEOUT";
				case VK_EVENT_SET: return "EVENT_SET";
				case VK_EVENT_RESET: return "EVENT_RESET";
				case VK_INCOMPLETE: return "INCOMPLETE";
				case VK_ERROR_OUT_OF_HOST_MEMORY: return "ERROR_OUT_OF_HOST_MEMORY";
				case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "ERROR_OUT_OF_DEVICE_MEMORY";
				case VK_ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
				case VK_ERROR_DEVICE_LOST: return "ERROR_DEVICE_LOST";
				case VK_ERROR_MEMORY_MAP_FAILED: return "ERROR_MEMORY_MAP_FAILED";
				case VK_ERROR_LAYER_NOT_PRESENT: return "ERROR_LAYER_NOT_PRESENT";
				case VK_ERROR_EXTENSION_NOT_PRESENT: return "ERROR_EXTENSION_NOT_PRESENT";
				case VK_ERROR_FEATURE_NOT_PRESENT: return "ERROR_FEATURE_NOT_PRESENT";
				case VK_ERROR_INCOMPATIBLE_DRIVER: return "ERROR_INCOMPATIBLE_DRIVER";
				case VK_ERROR_TOO_MANY_OBJECTS: return "ERROR_TOO_MANY_OBJECTS";
				case VK_ERROR_FORMAT_NOT_SUPPORTED: return "ERROR_FORMAT_NOT_SUPPORTED";
				case VK_ERROR_FRAGMENTED_POOL: return "ERROR_FRAGMENTED_POOL";
				case VK_ERROR_UNKNOWN: return "ERROR_UNKNOWN";
				case VK_ERROR_OUT_OF_POOL_MEMORY: return "ERROR_OUT_OF_POOL_MEMORY";
				case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "ERROR_INVALID_EXTERNAL_HANDLE";
				case VK_ERROR_FRAGMENTATION: return "ERROR_FRAGMENTATION";
				case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
				case VK_ERROR_SURFACE_LOST_KHR: return "ERROR_SURFACE_LOST_KHR";
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
				case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR";
				case VK_ERROR_OUT_OF_DATE_KHR: return "ERROR_OUT_OF_DATE_KHR";
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
				case VK_ERROR_VALIDATION_FAILED_EXT: return "ERROR_VALIDATION_FAILED_EXT";
				case VK_ERROR_INVALID_SHADER_NV: return "ERROR_INVALID_SHADER_NV";
				case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
				case VK_ERROR_NOT_PERMITTED_EXT: return "ERROR_NOT_PERMITTED_EXT";
				case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
				case VK_THREAD_IDLE_KHR: return "THREAD_IDLE_KHR";
				case VK_THREAD_DONE_KHR: return "THREAD_DONE_KHR";
				case VK_OPERATION_DEFERRED_KHR: return "OPERATION_DEFERRED_KHR";
				case VK_OPERATION_NOT_DEFERRED_KHR: return "OPERATION_NOT_DEFERRED_KHR";
				case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "PIPELINE_COMPILE_REQUIRED_EXT";
				default: return "Unknown VkResult";
			}
		}
		
		static void CheckVkResult(const char* str, const VkResult& res) {
			if (res == VK_SUCCESS) return;
			throw std::runtime_error(std::string(str) + ": " + GetVkResultText(res));
		}
		
	};
}
