/*
 * Vulkan Physical Device abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once

#include <v4d.h>
#include <vector>
#include <cstring>
#include <string>
#include "utilities/graphics/vulkan/Loader.h"

namespace v4d::graphics::vulkan {

	class V4DLIB PhysicalDevice {
	private:
		xvk::Interface::InstanceInterface* vulkanInstance;
		VkPhysicalDevice handle;

		VkPhysicalDeviceProperties deviceProperties {};
		
		std::vector<VkQueueFamilyProperties>* queueFamilies = nullptr;
		std::vector<VkExtensionProperties>* supportedExtensions = nullptr;

	public:
		PhysicalDevice(xvk::Interface::InstanceInterface* vulkanInstance, VkPhysicalDevice handle);
		~PhysicalDevice();
		
		class DeviceFeatures {
		public:
			VkPhysicalDeviceFeatures2 deviceFeatures2 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr, VkPhysicalDeviceFeatures{} };
			
			// Vulkan 1.1
			VkPhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR };
			VkPhysicalDevice16BitStorageFeatures _16bitStorageFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES };
			
			// Vulkan 1.2
			VkPhysicalDeviceVulkan12Features vulkan12DeviceFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
			VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
			VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
			
		private:
			const std::array<void*, 7> featuresPNextOrder {
				&deviceFeatures2,
				&shaderClockFeatures,
				&_16bitStorageFeatures,
				// Vulkan 1.2
				&vulkan12DeviceFeatures,
				&rayTracingPipelineFeatures,
				&accelerationStructureFeatures,
				&rayQueryFeatures,
			};
			
			template<typename T>
			static void AndFeatures(T* features, const T& otherFeatures, size_t offset = 0) {
				const size_t featuresArraySize = (sizeof(T)-offset) / sizeof(VkBool32);
				VkBool32 otherFeaturesData[featuresArraySize];
				VkBool32 featuresData[featuresArraySize];
				memcpy(otherFeaturesData, ((byte*)&otherFeatures)+offset, sizeof(otherFeaturesData));
				memcpy(featuresData, ((byte*)features)+offset, sizeof(featuresData));
				for (size_t i = 0; i < featuresArraySize; ++i) {
					featuresData[i] = (featuresData[i] && otherFeaturesData[i])? VK_TRUE : VK_FALSE;
				}
				memcpy(((byte*)features+offset), featuresData, sizeof(featuresData));
			}
		
		public:
		
			VkPhysicalDeviceFeatures2* GetDeviceFeaturesPNext() {
				for (size_t i = featuresPNextOrder.size(); i > 0; --i) {
					((VkBaseInStructure*)(featuresPNextOrder[i-1]))->pNext = (i==featuresPNextOrder.size()? nullptr: (VkBaseInStructure*)featuresPNextOrder[i] );
				}
				return &deviceFeatures2;
			}
			
			void operator &= (const DeviceFeatures& other) {
				AndFeatures(&deviceFeatures2.features, other.deviceFeatures2.features);
				AndFeatures(&shaderClockFeatures, other.shaderClockFeatures, sizeof(VkStructureType)+sizeof(void*));
				AndFeatures(&_16bitStorageFeatures, other._16bitStorageFeatures, sizeof(VkStructureType)+sizeof(void*));
				AndFeatures(&vulkan12DeviceFeatures, other.vulkan12DeviceFeatures, sizeof(VkStructureType)+sizeof(void*));
				AndFeatures(&rayTracingPipelineFeatures, other.rayTracingPipelineFeatures, sizeof(VkStructureType)+sizeof(void*));
				AndFeatures(&accelerationStructureFeatures, other.accelerationStructureFeatures, sizeof(VkStructureType)+sizeof(void*));
				AndFeatures(&rayQueryFeatures, other.rayQueryFeatures, sizeof(VkStructureType)+sizeof(void*));
			}
			
		} deviceFeatures {};

		// family queue index
		int GetQueueFamilyIndexFromFlags(VkQueueFlags flags, uint minQueuesCount = 1, VkSurfaceKHR* surface = nullptr);
		std::vector<int> GetQueueFamilyIndicesFromFlags(VkQueueFlags flags, uint minQueuesCount = 1, VkSurfaceKHR* surface = nullptr);
		bool QueueFamiliesContainsFlags(VkQueueFlags flags, uint minQueuesCount = 1, VkSurfaceKHR* surface = nullptr);

		bool SupportsExtension(std::string ext);

		VkPhysicalDeviceProperties GetProperties() const;
		
		VkPhysicalDevice GetHandle() const;
		xvk::Interface::InstanceInterface* GetVulkanInstance() const;
		std::string GetDescription() const;

		// create a logical device
		VkResult CreateDevice (const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);

		VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR (VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
		VkResult GetPhysicalDeviceSurfaceFormatsKHR (VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
		VkResult GetPhysicalDeviceSurfacePresentModesKHR (VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
		void GetPhysicalDeviceFormatProperties (VkFormat format, VkFormatProperties* pFormatProperties);

		uint FindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties);
		VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		VkSampleCountFlagBits GetMaxUsableSampleCount();

	};
}
