#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB DeviceQueueInfo {
		std::string name;
		VkDeviceQueueCreateFlags flags;
		uint count;
		std::vector<float> priorities;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};

	class V4DLIB Device : public xvk::Interface::DeviceInterface {
	private:
		PhysicalDevice* physicalDevice;

		VkDeviceCreateInfo createInfo {};
		VkDeviceQueueCreateInfo* queueCreateInfo;
		std::unordered_map<std::string, std::vector<Queue>> queues;

	public:
		Device(
			PhysicalDevice* physicalDevice,
			VkPhysicalDeviceFeatures& deviceFeatures,
			std::vector<const char*>& extensions,
			std::vector<const char*>& layers,
			const std::vector<DeviceQueueInfo>& queuesInfo
		);
		~Device();

		VkDevice GetHandle() const;
		VkPhysicalDevice GetPhysicalDeviceHandle() const;
		PhysicalDevice* GetPhysicalDevice() const;

		Queue GetPresentationQueue(VkSurfaceKHR surface, VkDeviceQueueCreateFlags flags = 0);
		Queue GetQueue(std::string name, uint index = 0) ;
		Queue GetQueue(uint queueFamilyIndex, uint index = 0);

		using xvk::Interface::DeviceInterface::CreateCommandPool;
		void CreateCommandPool(uint queueIndex, VkCommandPoolCreateFlags flags, VkCommandPool* commandPool);

		using xvk::Interface::DeviceInterface::DestroyCommandPool;
		void DestroyCommandPool(VkCommandPool &commandPool);

		using xvk::Interface::DeviceInterface::CreateDescriptorPool;
		void CreateDescriptorPool(std::vector<VkDescriptorType> types, uint32_t count, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);
		void CreateDescriptorPool(std::map<VkDescriptorType, uint>& types, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags = 0);

		using xvk::Interface::DeviceInterface::DestroyDescriptorPool;
		void DestroyDescriptorPool(VkDescriptorPool &descriptorPool);

		using xvk::Interface::DeviceInterface::CreateImage;
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkImage& image, VkDeviceMemory& imageMemory);

	};
}
