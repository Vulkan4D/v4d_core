#include "../v4dconfig.hh"

#ifdef V4D_VULKAN_USE_VMA
	#define XVK_INCLUDE_VMA
	#ifdef _V4D_CORE
		#define VMA_IMPLEMENTATION
	#endif
#endif

#include <v4d.h>

using namespace v4d::graphics::vulkan;

#ifndef XVK_USE_QT_VULKAN_LOADER

uint32_t Loader::VULKAN_API_VERSION = VK_API_VERSION_1_2;
uint32_t Loader::VULKAN_API_VERSION_ALTERNATIVE = VK_API_VERSION_1_1;

void Loader::CheckExtensions() {
	LOG_VERBOSE("Initializing Vulkan Extensions...");
	
	// Get supported extensions
	uint supportedExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());
	std::for_each(supportedExtensions.begin(), supportedExtensions.end(), [](const auto& extension){
		LOG_VERBOSE("Supported Vulkan Extension: " << extension.extensionName);
	});
	
	// Check support for required extensions
	for (const char* extensionName : requiredInstanceExtensions) {
		if (std::find_if(supportedExtensions.begin(), supportedExtensions.end(), [&extensionName](VkExtensionProperties extension){
			return strcmp(extension.extensionName, extensionName) == 0;
		}) != supportedExtensions.end()) {
			LOG("Enabling Vulkan Extension: " << extensionName);
		} else {
			throw std::runtime_error(std::string("Required Extension Not Supported: ") + extensionName);
		}
	}
}

void Loader::CheckLayers() {
	LOG_VERBOSE("Initializing Vulkan Layers...");
	
	// Get Supported Layers
	uint supportedLayerCount;
	vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
	std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
	vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());
	std::for_each(supportedLayers.begin(), supportedLayers.end(), [](const auto& layer){
		LOG_VERBOSE("Supported Layer: " << layer.layerName);
	});
	
	// Check support for required layers
	for (const char* layerName : requiredInstanceLayers) {
		if (std::find_if(supportedLayers.begin(), supportedLayers.end(), [&layerName](VkLayerProperties layer){
			return strcmp(layer.layerName, layerName) == 0;
		}) != supportedLayers.end()) {
			LOG("Enabling Vulkan Layer: " << layerName);
		} else {
			throw std::runtime_error(std::string("Layer Not Supported: ") + layerName);
		}
	}
}

void Loader::CheckVulkanVersion() {
	uint32_t apiVersion = 0;
	vkEnumerateInstanceVersion(&apiVersion);
	if (apiVersion < VULKAN_API_VERSION) {
		uint vMajor = (VULKAN_API_VERSION & (511 << 22)) >> 22;
		uint vMinor = (VULKAN_API_VERSION & (1023 << 12)) >> 12;
		uint vPatch = VULKAN_API_VERSION & 4095;
		throw std::runtime_error("Vulkan Version " + std::to_string(vMajor) + "." + std::to_string(vMinor) + "." + std::to_string(vPatch) + " is Not supported");
	}
}

#endif
