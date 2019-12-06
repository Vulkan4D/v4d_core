#include <v4d.h>

using namespace v4d::graphics::vulkan;

Device::Device(
	PhysicalDevice* physicalDevice,
	VkPhysicalDeviceFeatures& deviceFeatures,
	std::vector<const char*>& extensions,
	std::vector<const char*>& layers,
	std::vector<DeviceQueueInfo> queuesInfo
) : physicalDevice(physicalDevice) {
	instance = physicalDevice->GetVulkanInstance();

	// Queues
	std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo {};
	queuesCreateInfo.reserve(queuesInfo.size());
	for (auto& queueInfo : queuesInfo) {
		if (queueInfo.count != queueInfo.priorities.size()) {
			throw std::runtime_error("Queue priorities list size does not match queue count");
		}
		queues[queueInfo.name] = std::vector<Queue>(queueInfo.count);
		bool foundQueueFamilyIndex = false;
		auto queueFamilyIndices = physicalDevice->GetQueueFamilyIndicesFromFlags(queueInfo.flags, queueInfo.count, queueInfo.surface);
		for (auto queueFamilyIndex : queueFamilyIndices) {
			// If queueFamilyIndex is not already present in our queuesCreateInfo vector, add it
			auto existing = std::find_if(queuesCreateInfo.begin(), queuesCreateInfo.end(), [queueFamilyIndex](VkDeviceQueueCreateInfo& n){return n.queueFamilyIndex == queueFamilyIndex;});
			if (existing == queuesCreateInfo.end()) {
				queueInfo.queueFamilyIndex = queueFamilyIndex;
				queueInfo.indexOffset = 0;
				queueInfo.createInfoIndex = queuesCreateInfo.size();
				queuesCreateInfo.push_back({
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType sType
					nullptr, // const void* pNext
					0, // VkDeviceQueueCreateFlags flags
					(uint)queueFamilyIndex, // uint32_t queueFamilyIndex
					queueInfo.count, // uint32_t queueCount
					queueInfo.priorities.data(), // const float* pQueuePriorities
				});
				foundQueueFamilyIndex = true;
				break;
			}
		}
		if (!foundQueueFamilyIndex) {
			// If could not find a different queue family, try to use an existing one
			for (auto& existingQueuesInfo : queuesInfo) {
				if (existingQueuesInfo.createInfoIndex == -1) break;
				auto& existingQueueCreateInfo = queuesCreateInfo[existingQueuesInfo.createInfoIndex];
				if (existingQueueCreateInfo.queueFamilyIndex == physicalDevice->GetQueueFamilyIndexFromFlags(queueInfo.flags, queueInfo.count + existingQueueCreateInfo.queueCount, queueInfo.surface)) {
					queueInfo.queueFamilyIndex = existingQueueCreateInfo.queueFamilyIndex;
					queueInfo.indexOffset = existingQueueCreateInfo.queueCount;
					queueInfo.createInfoIndex = -1;
					existingQueueCreateInfo.queueCount += queueInfo.count;
					for (auto f : queueInfo.priorities) existingQueuesInfo.priorities.push_back(f);
					existingQueueCreateInfo.pQueuePriorities = existingQueuesInfo.priorities.data();
					foundQueueFamilyIndex = true;
					break;
				}
			}
		}
		if (!foundQueueFamilyIndex) {
			throw std::runtime_error("Failed to find a suitable queue family for " + queueInfo.name);
		}
	}
	
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queuesCreateInfo.size();
	createInfo.pQueueCreateInfos = queuesCreateInfo.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();

	if (physicalDevice->CreateDevice(&createInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}
	
	LoadFunctionPointers();

	// Get Queues Handles
	for (auto queueInfo : queuesInfo) {
		queues[queueInfo.name] = std::vector<Queue>(queueInfo.count);
		for (uint i = 0; i < queueInfo.count; i++) {
			auto *q = &queues[queueInfo.name][i];
			GetDeviceQueue(queueInfo.queueFamilyIndex, queueInfo.indexOffset + i, &(q->handle));
			q->familyIndex = (uint)queueInfo.queueFamilyIndex;
		}
	}
}

Device::~Device() {
	DeviceWaitIdle();
	DestroyDevice(nullptr);
	handle = VK_NULL_HANDLE;
}

VkDevice Device::GetHandle() const {
	return handle;
}

VkPhysicalDevice Device::GetPhysicalDeviceHandle() const {
	return physicalDevice->GetHandle();
}

PhysicalDevice* Device::GetPhysicalDevice() const {
	return physicalDevice;
}

Queue Device::GetPresentationQueue(VkSurfaceKHR surface, VkDeviceQueueCreateFlags flags) {
	return GetQueue(physicalDevice->GetQueueFamilyIndexFromFlags(flags, 1, surface));
}

Queue Device::GetQueue(std::string name, uint index) {
	return queues[name][index];
}

Queue Device::GetQueue(uint queueFamilyIndex, uint index) {
	Queue q = {queueFamilyIndex, nullptr};
	GetDeviceQueue(queueFamilyIndex, index, &q.handle);
	return q;
}

void Device::CreateCommandPool(uint queueIndex, VkCommandPoolCreateFlags flags, VkCommandPool* commandPool) {
	// Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved.
	// Each command pool can only allocate command buffers that are submitted on a single type of queue.
	// We're going to record commands for drawing, which is why we've choosen the graphics queue family.
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueIndex;
	commandPoolCreateInfo.flags = flags; // We will only record the command buffers at the beginning of the program and then execute them many times in the main loop, so we're not going to use flags here
							/*	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
								VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
							*/
	if (CreateCommandPool(&commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool");
	}
}

void Device::DestroyCommandPool(VkCommandPool &commandPool) {
	DestroyCommandPool(commandPool, nullptr);
}

void Device::CreateDescriptorPool(std::vector<VkDescriptorType> types, uint32_t count, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags) {
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(types.size());
	for (auto type : types) {
		poolSizes.emplace_back(VkDescriptorPoolSize{type, count});
	}
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = count;
	poolInfo.flags = flags;
	if (CreateDescriptorPool(&poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool");
	}
}
void Device::CreateDescriptorPool(std::map<VkDescriptorType, uint>& types, VkDescriptorPool& descriptorPool, VkDescriptorPoolCreateFlags flags) {
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(types.size());
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = flags;
	poolInfo.maxSets = 0;
	for (auto [type, count] : types) {
		poolSizes.emplace_back(VkDescriptorPoolSize{type, count});
		poolInfo.maxSets += count;
	}
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	if (CreateDescriptorPool(&poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void Device::DestroyDescriptorPool(VkDescriptorPool &descriptorPool) {
	DestroyDescriptorPool(descriptorPool, nullptr);
}

void Device::CreateImage(uint32_t width, 
	uint32_t height, 
	uint32_t mipLevels, 
	VkSampleCountFlagBits sampleCount, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags memoryPropertyFlags, 
	VkImage& image, 
	VkDeviceMemory& imageMemory, 
	uint32_t arrayLayers, 
	VkImageCreateFlags flags,
	std::vector<uint32_t> queues
) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = sampleCount;
	imageInfo.flags = flags;
	
	if (queues.size() > 2 || (queues.size() == 2 && queues[0] != queues[1])) {
		imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	} else {
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	imageInfo.queueFamilyIndexCount = queues.size();
	imageInfo.pQueueFamilyIndices = queues.data();

	if (CreateImage(&imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create depth image");
	}

	VkMemoryRequirements memRequirements;
	GetImageMemoryRequirements(image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = physicalDevice->FindMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);

	if (AllocateMemory(&allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory");
	}

	BindImageMemory(image, imageMemory, 0);
}


size_t Device::GetAlignedUniformSize(size_t size) {
	size_t alignedSize = size;
	const VkDeviceSize& alignment = physicalDevice->GetProperties().limits.minUniformBufferOffsetAlignment;
	if (alignedSize % alignment) alignedSize += alignment - (alignedSize % alignment);
	return alignedSize;
}
