#include "PhysicalDevice.h"
#include "utilities/io/Logger.h"

using namespace v4d::graphics::vulkan;

PhysicalDevice::PhysicalDevice(xvk::Interface::InstanceInterface* vulkanInstance, VkPhysicalDevice handle) : vulkanInstance(vulkanInstance), handle(handle) {
	// Properties
	vulkanInstance->GetPhysicalDeviceProperties(handle, &deviceProperties);
	LOG_VERBOSE("DETECTED PhysicalDevice: " << deviceProperties.deviceName);
	
	// VRAM
	vulkanInstance->GetPhysicalDeviceMemoryProperties(handle, &deviceMemoryProperties);
	std::vector<VkMemoryHeap> heaps (deviceMemoryProperties.memoryHeaps, deviceMemoryProperties.memoryHeaps + deviceMemoryProperties.memoryHeapCount);
	for (auto& heap : heaps) {
		if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			totalVRAM += heap.size;
		}
	}
	
	// Supported Extensions
	uint supportedExtensionsCount = 0;
	vulkanInstance->EnumerateDeviceExtensionProperties(handle, nullptr, &supportedExtensionsCount, nullptr);
	supportedExtensions = new std::vector<VkExtensionProperties>(supportedExtensionsCount);
	vulkanInstance->EnumerateDeviceExtensionProperties(handle, nullptr, &supportedExtensionsCount, supportedExtensions->data());
	
	std::for_each(supportedExtensions->begin(), supportedExtensions->end(), [](const auto& extension){
		LOG_VERBOSE("Supported Device Extension: " << extension.extensionName);
	});
	
	// Get supported Features
	vulkanInstance->GetPhysicalDeviceFeatures2(handle, deviceFeatures.GetDeviceFeaturesPNext());
	
	// Queue Families
	uint queueFamilyCount = 0;
	vulkanInstance->GetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, nullptr);
	queueFamilies = new std::vector<VkQueueFamilyProperties>(queueFamilyCount);
	vulkanInstance->GetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, queueFamilies->data());
}

PhysicalDevice::~PhysicalDevice() {
	delete queueFamilies;
	delete supportedExtensions;
}

int PhysicalDevice::GetQueueFamilyIndexFromFlags(VkQueueFlags flags, uint minQueuesCount, VkSurfaceKHR* surface) {
	int i = 0;
	// Prioritize a specialized queue family
	std::vector<VkQueueFlags> specializedFlags {
		flags,
		flags|VK_QUEUE_SPARSE_BINDING_BIT,
		flags|VK_QUEUE_TRANSFER_BIT,
		flags|VK_QUEUE_SPARSE_BINDING_BIT|VK_QUEUE_TRANSFER_BIT,
	};
	for (auto f : specializedFlags) {
		for (const auto& queueFamily : *queueFamilies) {
			if (queueFamily.queueCount >= minQueuesCount && (queueFamily.queueFlags == f)) {
				if (!surface || *surface == VK_NULL_HANDLE) return i;
				VkBool32 presentationSupport;
				vulkanInstance->GetPhysicalDeviceSurfaceSupportKHR(handle, i, *surface, &presentationSupport);
				if (presentationSupport) return i;
			}
			i++;
		}
		i = 0;
	}
	// no specialized queue family, simply take the first that is compatible
	for (const auto& queueFamily : *queueFamilies) {
		if (queueFamily.queueCount >= minQueuesCount && (queueFamily.queueFlags & flags) == flags) {
			if (!surface || *surface == VK_NULL_HANDLE) return i;
			VkBool32 presentationSupport;
			vulkanInstance->GetPhysicalDeviceSurfaceSupportKHR(handle, i, *surface, &presentationSupport);
			if (presentationSupport) return i;
		}
		i++;
	}
	return -1;
}

std::vector<int> PhysicalDevice::GetQueueFamilyIndicesFromFlags(VkQueueFlags flags, uint minQueuesCount, VkSurfaceKHR* surface) {
	std::vector<int> indices {};
	int i = 0;
	for (const auto& queueFamily : *queueFamilies) {
		if (queueFamily.queueCount >= minQueuesCount && (queueFamily.queueFlags & flags) == flags) {
			if (!surface || *surface == VK_NULL_HANDLE) {
				indices.push_back(i);
			} else {
				VkBool32 presentationSupport;
				vulkanInstance->GetPhysicalDeviceSurfaceSupportKHR(handle, i, *surface, &presentationSupport);
				if (presentationSupport) indices.push_back(i);
			}
		}
		i++;
	}
	// Prioritize a specialized queue family
	std::sort(indices.begin(), indices.end(), [this](int a, int b){
		if (queueFamilies->at(a).queueFlags == queueFamilies->at(b).queueFlags) return a < b;
		return queueFamilies->at(a).queueFlags < queueFamilies->at(b).queueFlags;
	});
	return indices;
}

bool PhysicalDevice::HasSpecializedTransferQueue() const {
	for (const auto& queueFamily : *queueFamilies) {
		if (queueFamily.queueFlags == VK_QUEUE_TRANSFER_BIT || queueFamily.queueFlags == (VK_QUEUE_TRANSFER_BIT|VK_QUEUE_SPARSE_BINDING_BIT)) {
			return true;
		}
	}
	return false;
}

bool PhysicalDevice::HasSpecializedComputeQueue() const {
	for (const auto& queueFamily : *queueFamilies) {
		if (queueFamily.queueFlags == VK_QUEUE_COMPUTE_BIT || queueFamily.queueFlags == (VK_QUEUE_COMPUTE_BIT|VK_QUEUE_SPARSE_BINDING_BIT) || queueFamily.queueFlags == (VK_QUEUE_COMPUTE_BIT|VK_QUEUE_TRANSFER_BIT|VK_QUEUE_SPARSE_BINDING_BIT)) {
			return true;
		}
	}
	return false;
}

bool PhysicalDevice::QueueFamiliesContainsFlags(VkQueueFlags flags, uint minQueuesCount, VkSurfaceKHR* surface) {
	for (const auto& queueFamily : *queueFamilies) {
		if (queueFamily.queueCount >= minQueuesCount && (queueFamily.queueFlags & flags) == flags) {
			if (!surface || *surface == VK_NULL_HANDLE) return true;
			VkBool32 presentationSupport;
			vulkanInstance->GetPhysicalDeviceSurfaceSupportKHR(handle, 0, *surface, &presentationSupport);
			if (presentationSupport) return true;
		}
	}
	return false;
}

bool PhysicalDevice::SupportsExtension(std::string ext) {
	for (const auto& extension : *supportedExtensions) {
		if (extension.extensionName == ext) {
			return true;
		}
	}
	return false;
}

VkPhysicalDeviceProperties PhysicalDevice::GetProperties() const {
	return deviceProperties;
}

VkPhysicalDevice PhysicalDevice::GetHandle() const {
	return handle;
}

xvk::Interface::InstanceInterface* PhysicalDevice::GetVulkanInstance() const {
	return vulkanInstance;
}

VkResult PhysicalDevice::CreateDevice (const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
	return vulkanInstance->CreateDevice(handle, pCreateInfo, pAllocator, pDevice);
}

std::string PhysicalDevice::GetDescription() const {
	return GetProperties().deviceName;
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfaceCapabilitiesKHR (VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
	return vulkanInstance->GetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, pSurfaceCapabilities);
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfaceFormatsKHR (VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
	return vulkanInstance->GetPhysicalDeviceSurfaceFormatsKHR(handle, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfacePresentModesKHR (VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
	return vulkanInstance->GetPhysicalDeviceSurfacePresentModesKHR(handle, surface, pPresentModeCount, pPresentModes);
}

void PhysicalDevice::GetPhysicalDeviceFormatProperties (VkFormat format, VkFormatProperties* pFormatProperties) {
	vulkanInstance->GetPhysicalDeviceFormatProperties(handle, format, pFormatProperties);
}

uint PhysicalDevice::FindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vulkanInstance->GetPhysicalDeviceMemoryProperties(handle, &memProperties);
	for (uint i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type");
}

VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vulkanInstance->GetPhysicalDeviceFormatProperties(handle, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("Failed to find supported format");
}

VkSampleCountFlagBits PhysicalDevice::GetMaxUsableSampleCount() {
	VkSampleCountFlags counts = std::min(GetProperties().limits.framebufferColorSampleCounts, GetProperties().limits.framebufferDepthSampleCounts);
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}
