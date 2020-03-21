/*
 * Vulkan Logical Device abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This file also includes helper structs Queue and DeviceQueueInfo
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB Queue {
		uint32_t familyIndex;
		VkQueue handle = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
	};

	struct V4DLIB DeviceQueueInfo {
		std::string name;
		VkDeviceQueueCreateFlags flags;
		uint count;
		std::vector<float> priorities;
		VkSurfaceKHR surface;
		DeviceQueueInfo(
			std::string name,
			VkDeviceQueueCreateFlags flags,
			uint count = 1,
			std::vector<float> priorities = {1.0f},
			VkSurfaceKHR surface = VK_NULL_HANDLE
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

	class V4DLIB Device : public xvk::Interface::DeviceInterface {
	private:
		PhysicalDevice* physicalDevice;
		VkDeviceCreateInfo createInfo {};
		std::unordered_map<std::string, std::vector<Queue>> queues;

	public:
		Device(
			PhysicalDevice* physicalDevice,
			VkPhysicalDeviceFeatures& deviceFeatures,
			std::vector<const char*>& extensions,
			std::vector<const char*>& layers,
			std::vector<DeviceQueueInfo> queuesInfo
		);
		~Device();

		VkDevice GetHandle() const;
		VkPhysicalDevice GetPhysicalDeviceHandle() const;
		PhysicalDevice* GetPhysicalDevice() const;

		Queue GetPresentationQueue(VkSurfaceKHR surface, VkDeviceQueueCreateFlags flags = 0);
		Queue GetQueue(std::string name, uint index = 0) ;
		Queue GetQueue(uint queueFamilyIndex, uint index = 0);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::CreateCommandPool;
		void CreateCommandPool(uint queueIndex, VkCommandPoolCreateFlags flags, VkCommandPool* commandPool);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::DestroyCommandPool;
		void DestroyCommandPool(VkCommandPool&);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::CreateDescriptorPool;
		void CreateDescriptorPool(std::vector<VkDescriptorType> types, uint32_t count, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);
		void CreateDescriptorPool(std::map<VkDescriptorType, uint>& types, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::DestroyDescriptorPool;
		void DestroyDescriptorPool(VkDescriptorPool&);

		// overloads native vulkan command with different arguments
		using xvk::Interface::DeviceInterface::GetBufferDeviceAddress;
		VkDeviceAddress GetBufferDeviceAddress(VkBuffer&);
		VkDeviceOrHostAddressKHR GetBufferDeviceOrHostAddress(VkBuffer&);
		VkDeviceOrHostAddressConstKHR GetBufferDeviceOrHostAddressConst(VkBuffer&);

		// Helpers
		size_t GetAlignedUniformSize(size_t size);

		VkCommandBuffer BeginSingleTimeCommands(Queue queue);
		void EndSingleTimeCommands(Queue queue, VkCommandBuffer commandBuffer);
		
	};
}
