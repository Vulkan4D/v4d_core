#include "Instance.h"
#include "utilities/io/Logger.h"

using namespace v4d::graphics::vulkan;

#ifdef XVK_USE_QT_VULKAN_LOADER
	Instance::Instance(vulkan::Loader* loader) : vulkanLoader(loader) {
		this->loader = loader;
		
		// Create the Vulkan instance
		if (!vulkanLoader->qtVulkanInstance->create())
			throw std::runtime_error("Failed to create Vulkan Instance");
		handle = vulkanLoader->qtVulkanInstance->vkInstance();

		LoadFunctionPointers();

		// Load Physical Devices
		LoadAvailablePhysicalDevices();
	}
#else

	// Debug Callback
	#ifdef V4D_VULKAN_USE_VALIDATION_LAYERS
		VkDebugUtilsMessengerEXT vulkanCallbackExtFunction;
		static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* /*pUserData*/) {
				std::string type;
				switch (messageType) {
					default:
					case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: // Some event has happened that is unrelated to the specification or performance
						type = "";
					break;
					case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: // Something has happened that violates the specification or indicates a possible mistake
						type = "(validation)";
					break;
					case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: // Potential non-optimal use of Vulkan
						type = "(performance)";
					break;
				}
				switch (messageSeverity) {
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: // Diagnostic message
						LOG_VERBOSE("VULKAN_VERBOSE" << type << ": " << pCallbackData->pMessage);
					break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: // Informational message like the creation of a resource
						LOG_VERBOSE("VULKAN_INFO" << type << ": " << pCallbackData->pMessage);
					break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: // Message about behavior that is not necessarily an error, but very likely a bug in your application
						LOG_WARN("VULKAN_WARNING" << type << ": " << pCallbackData->pMessage);
						// std::abort();
					break;
					default:
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: // Message about behavior that is invalid and may cause crashes
					
						// Ignored errors
						if (std::string("VUID-VkSwapchainCreateInfoKHR-imageExtent-01274") == pCallbackData->pMessageIdName) break;
						
						LOG_ERROR("VULKAN_ERROR" << type << ": " << pCallbackData->pMessage);
						#ifndef _WINDOWS
							#ifdef V4D_VULKAN_VALIDATION_ABORT_ON_ERROR
								std::abort();
							#endif
						#endif
					break;
				}
				return VK_FALSE; // The callback returns a boolean that indicates if the Vulkan call that triggered the validation layer message should be aborted. If the callback returns true, then the call is aborted with the VK_ERROR_VALIDATION_FAILED_EXT error. This is normally only used to test the validation layers themselves, so you should always return VK_FALSE.
		}
	#endif

	Instance::Instance(vulkan::Loader* loader, const char* applicationName, uint applicationVersion) : vulkanLoader(loader) {
		this->loader = loader;
		
		// Check extensions and layers
		loader->CheckLayers();
		loader->CheckExtensions();
		loader->CheckVulkanVersion();

		// Prepare appInfo for the Vulkan Instance
		VkApplicationInfo appInfo {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr, //pNext
			applicationName,
			applicationVersion,
			V4D_ENGINE_NAME,
			V4D_ENGINE_VERSION,
			vulkan::Loader::VULKAN_API_VERSION
		};

		// Create the Vulkan instance
		VkInstanceCreateInfo createInfo {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr, // pNext
			0, // flags
			&appInfo,
			(uint32_t)loader->requiredInstanceLayers.size(),
			loader->requiredInstanceLayers.data(),
			(uint32_t)loader->requiredInstanceExtensions.size(),
			loader->requiredInstanceExtensions.data()
		};
		if (vulkanLoader->vkCreateInstance(&createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan Instance");
			
		LoadFunctionPointers();

		// Debug Callback
		#ifdef V4D_VULKAN_USE_VALIDATION_LAYERS
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				nullptr,// pNext
				0,// flags
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, // messageSeverity
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, // messageType
				vulkanDebugCallback,// pfnUserCallback
				nullptr,// pUserData
			};
			if (false) { // log verbose
				debugUtilsMessengerCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
				debugUtilsMessengerCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
			}
			if (CreateDebugUtilsMessengerEXT(&debugUtilsMessengerCreateInfo, nullptr, &vulkanCallbackExtFunction) != VK_SUCCESS) {
				std::cout << "Failed to set Vulkan Debug Callback Function" << std::endl;
			}
		#endif

		// Load Physical Devices
		LoadAvailablePhysicalDevices();
		
		LOG_VERBOSE("Vulkan instance created");
	}
#endif

Instance::~Instance() {
	#ifdef V4D_VULKAN_USE_VALIDATION_LAYERS
		DestroyDebugUtilsMessengerEXT(vulkanCallbackExtFunction, nullptr);
	#endif
	#ifdef XVK_USE_QT_VULKAN_LOADER
		vulkanLoader->qtVulkanInstance->destroy();
	#else
		DestroyInstance(nullptr);
	#endif
	for (auto *physicalDevice : availablePhysicalDevices) {
		delete physicalDevice;
	}
}

void Instance::LoadAvailablePhysicalDevices() {
	uint vMajor = (Loader::VULKAN_API_VERSION & (511 << 22)) >> 22;
	uint vMinor = (Loader::VULKAN_API_VERSION & (1023 << 12)) >> 12;
	uint vPatch = Loader::VULKAN_API_VERSION & 4095;
	LOG_VERBOSE("Initializing Physical Devices supporting Vulkan " << vMajor << "." << vMinor << "." << vPatch);
	
	// Get Devices List
	uint physicalDeviceCount = 0;
	EnumeratePhysicalDevices(&physicalDeviceCount, nullptr);
	if (physicalDeviceCount == 0) throw std::runtime_error("Failed to find a PhysicalDevice with Vulkan Support");
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	EnumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.data());
	
	// Add detected physicalDevices to our own physical device list
	availablePhysicalDevices.reserve(physicalDeviceCount);
	for (const auto& physicalDevice : physicalDevices) {
		VkPhysicalDeviceProperties properties = {};
		GetPhysicalDeviceProperties(physicalDevice, &properties);
		if (properties.apiVersion >= Loader::VULKAN_API_VERSION)
			availablePhysicalDevices.push_back(new PhysicalDevice(this, physicalDevice));
	}
	
	// Error if no PhysicalDevice was found (or try alternative vulkan version)
	if (availablePhysicalDevices.size() == 0) {
		if (Loader::VULKAN_API_VERSION_ALTERNATIVE && Loader::VULKAN_API_VERSION_ALTERNATIVE != Loader::VULKAN_API_VERSION) {
			Loader::VULKAN_API_VERSION = Loader::VULKAN_API_VERSION_ALTERNATIVE;
			Loader::VULKAN_API_VERSION_ALTERNATIVE = 0;
			LoadAvailablePhysicalDevices();
			return;
		}
		throw std::runtime_error("No suitable PhysicalDevice was found for required vulkan version");
	}
}

VkInstance Instance::GetHandle() const {
	return handle;
}

PhysicalDevice* Instance::SelectSuitablePhysicalDevice(const std::function<void(int&, PhysicalDevice*)>& suitabilityFunc) {
	PhysicalDevice* selectedPhysicalDevice = nullptr;

	// Select Best MainPhysicalDevice
	int bestScore = 0;
	for (auto* physicalDevice : availablePhysicalDevices) {
		int score = 0;

		suitabilityFunc(score, physicalDevice);

		if (score > 0 && score > bestScore){
			selectedPhysicalDevice = physicalDevice;
			bestScore = score;
		}
	}

	// Make sure that all mandatory PhysicalDevices have been selected
	if (selectedPhysicalDevice == nullptr) throw std::runtime_error("Failed to find a suitable PhysicalDevice");
	return selectedPhysicalDevice;
}
