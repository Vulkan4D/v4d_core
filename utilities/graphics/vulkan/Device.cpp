#include "Device.h"

using namespace v4d::graphics::vulkan;

Device::Device(
	PhysicalDevice* physicalDevice,
	std::vector<const char*>& extensions,
	std::vector<const char*>& layers,
	std::vector<DeviceQueueInfo> queuesInfo,
	void* pNext
) : physicalDevice(physicalDevice) {
	instance = physicalDevice->GetVulkanInstance();

	// Queues
	std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo {};
	queuesCreateInfo.reserve(queuesInfo.size());
	for (auto& queueInfo : queuesInfo) {
		if (queueInfo.count != queueInfo.priorities.size()) {
			throw std::runtime_error("Queue priorities list size does not match queue count");
		}
		queues[queueInfo.flags] = std::vector<Queue>(queueInfo.count);
		bool foundQueueFamilyIndex = false;
		auto queueFamilyIndices = physicalDevice->GetQueueFamilyIndicesFromFlags(queueInfo.flags, queueInfo.count, queueInfo.surface);
		for (auto queueFamilyIndex : queueFamilyIndices) {
			// If queueFamilyIndex is not already present in our queuesCreateInfo vector, add it
			auto existing = std::find_if(queuesCreateInfo.begin(), queuesCreateInfo.end(), [queueFamilyIndex](VkDeviceQueueCreateInfo& n){return n.queueFamilyIndex == (uint32_t)queueFamilyIndex;});
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
				if (existingQueueCreateInfo.queueFamilyIndex == (uint32_t)physicalDevice->GetQueueFamilyIndexFromFlags(queueInfo.flags, queueInfo.count + existingQueueCreateInfo.queueCount, queueInfo.surface)) {
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
			throw std::runtime_error("Failed to find a suitable queue family for " + std::to_string(queueInfo.flags));
		}
	}
	
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.queueCreateInfoCount = queuesCreateInfo.size();
	createInfo.pQueueCreateInfos = queuesCreateInfo.data();
	createInfo.pEnabledFeatures = nullptr;
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
		queues[queueInfo.flags] = std::vector<Queue>(queueInfo.count);
		for (uint i = 0; i < queueInfo.count; i++) {
			auto *q = &queues[queueInfo.flags][i];
			GetDeviceQueue(queueInfo.queueFamilyIndex, queueInfo.indexOffset + i, &(q->handle));
			q->familyIndex = (uint)queueInfo.queueFamilyIndex;
		}
		
		// Assign a present queue to the one with a Surface
		if (queueInfo.surface && *queueInfo.surface != VK_NULL_HANDLE) {
			queues[0] = queues[queueInfo.flags];
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

VkDeviceAddress Device::GetBufferDeviceAddress(const VkBuffer& buffer) {
	VkBufferDeviceAddressInfo bufferAddrInfo {};
	bufferAddrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferAddrInfo.buffer = buffer;
	return GetBufferDeviceAddress(&bufferAddrInfo);
}

VkDeviceOrHostAddressKHR Device::GetBufferDeviceOrHostAddress(const VkBuffer& buffer) {
	VkBufferDeviceAddressInfo bufferAddrInfo {};
	bufferAddrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferAddrInfo.buffer = buffer;
	VkDeviceOrHostAddressKHR bufferAddr {};
	bufferAddr.deviceAddress = GetBufferDeviceAddress(&bufferAddrInfo);
	return bufferAddr;
}

VkDeviceOrHostAddressConstKHR Device::GetBufferDeviceOrHostAddressConst(const VkBuffer& buffer) {
	VkBufferDeviceAddressInfo bufferAddrInfo {};
	bufferAddrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferAddrInfo.buffer = buffer;
	VkDeviceOrHostAddressConstKHR bufferAddr {};
	bufferAddr.deviceAddress = GetBufferDeviceAddress(&bufferAddrInfo);
	return bufferAddr;
}

size_t Device::GetAlignedUniformSize(size_t size) {
	size_t alignedSize = size;
	const VkDeviceSize& alignment = physicalDevice->GetProperties().limits.minUniformBufferOffsetAlignment;
	if (alignedSize % alignment) alignedSize += alignment - (alignedSize % alignment);
	return alignedSize;
}

VkCommandBuffer Device::BeginSingleTimeCommands(Queue queue) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = queue.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	AllocateCommandBuffers(&allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	BeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Device::EndSingleTimeCommands(Queue queue, VkCommandBuffer commandBuffer) {
	EndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0;
	VkFence fence;
	if (CreateFence(&fenceInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create fence");

	if (QueueSubmit(queue.handle, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit queue");
	
	// #ifdef _LINUX
	// 	uint64_t timeout = 1000UL * 1000 * 1000 * 10; /* 10 seconds in nanoseconds */
	// #else
		uint64_t timeout = std::numeric_limits<uint64_t>::max();
	// #endif
	if (VkResult res = WaitForFences(1, &fence, VK_TRUE, timeout); res != VK_SUCCESS) {
		// LOG_ERROR(res)
		throw std::runtime_error("Failed to wait for fence to signal");
	}

	DestroyFence(fence, nullptr);
	
	FreeCommandBuffers(queue.commandPool, 1, &commandBuffer);
	
	// QueueWaitIdle(queue.handle);
}

void Device::RunSingleTimeCommands(Queue queue, std::function<void(VkCommandBuffer)>&& func) {
	auto cmdBuffer = BeginSingleTimeCommands(queue);
	func(cmdBuffer);
	EndSingleTimeCommands(queue, cmdBuffer);
}

// Allocator
void Device::CreateAllocator() {
	#ifdef V4D_VULKAN_USE_VMA
		VmaVulkanFunctions vulkanFunctions {};{
			vulkanFunctions.vkGetPhysicalDeviceProperties = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetPhysicalDeviceProperties;
			vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetPhysicalDeviceMemoryProperties;
			vulkanFunctions.vkAllocateMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkAllocateMemory;
			vulkanFunctions.vkFreeMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkFreeMemory;
			vulkanFunctions.vkMapMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkMapMemory;
			vulkanFunctions.vkUnmapMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkUnmapMemory;
			vulkanFunctions.vkFlushMappedMemoryRanges = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkFlushMappedMemoryRanges;
			vulkanFunctions.vkInvalidateMappedMemoryRanges = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkInvalidateMappedMemoryRanges;
			vulkanFunctions.vkBindBufferMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkBindBufferMemory;
			vulkanFunctions.vkBindImageMemory = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkBindImageMemory;
			vulkanFunctions.vkGetBufferMemoryRequirements = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetBufferMemoryRequirements;
			vulkanFunctions.vkGetImageMemoryRequirements = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetImageMemoryRequirements;
			vulkanFunctions.vkCreateBuffer = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkCreateBuffer;
			vulkanFunctions.vkDestroyBuffer = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkDestroyBuffer;
			vulkanFunctions.vkCreateImage = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkCreateImage;
			vulkanFunctions.vkDestroyImage = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkDestroyImage;
			vulkanFunctions.vkCmdCopyBuffer = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkCmdCopyBuffer;
			#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
				vulkanFunctions.vkGetBufferMemoryRequirements2KHR = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetBufferMemoryRequirements2KHR;
				vulkanFunctions.vkGetImageMemoryRequirements2KHR = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetImageMemoryRequirements2KHR;
			#endif
			#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
				vulkanFunctions.vkBindBufferMemory2KHR = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkBindBufferMemory2KHR;
				vulkanFunctions.vkBindImageMemory2KHR = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkBindImageMemory2KHR;
			#endif
			#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
				vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE::vkGetPhysicalDeviceMemoryProperties2KHR;
			#endif
		}
		VmaAllocatorCreateInfo allocatorInfo {};{
			allocatorInfo.vulkanApiVersion = Loader::VULKAN_API_VERSION;
			allocatorInfo.physicalDevice = physicalDevice->GetHandle();
			allocatorInfo.device = handle;
			allocatorInfo.instance = instance->handle;
			allocatorInfo.frameInUseCount = 1;
			allocatorInfo.pVulkanFunctions = &vulkanFunctions;
			if (Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2) {
				allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT;
			}
		}
		vmaCreateAllocator(&allocatorInfo, &allocator);
	#endif
}
void Device::DestroyAllocator() {
	#ifdef V4D_VULKAN_USE_VMA
		std::lock_guard lock(allocatorDeleteMutex);
		vmaDestroyAllocator(allocator);
		allocator = VK_NULL_HANDLE;
	#endif
}
VkResult Device::CreateAndAllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, MemoryUsage memoryUsage, VkBuffer& buffer, MemoryAllocation* pAllocation, bool weakAllocation) {
	#ifdef V4D_VULKAN_USE_VMA
		VmaAllocationCreateInfo allocInfo {};
			allocInfo.usage = (VmaMemoryUsage)memoryUsage;
			if (weakAllocation && memoryUsage == MEMORY_USAGE_GPU_ONLY) {
				allocInfo.flags |= VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT | VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
			}
			if (Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2 && (bufferCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
				allocInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			}
		return vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &buffer, pAllocation, nullptr);
	#else
		if (CreateBuffer(&bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create buffer");
		}

		VkMemoryRequirements memRequirements;
		GetBufferMemoryRequirements(buffer, &memRequirements);
		
		VkDeviceSize allocSize = memRequirements.size;
		if ((allocSize % memRequirements.alignment) > 0) {
			allocSize += memRequirements.alignment - (allocSize % memRequirements.alignment);
		}
		
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo {};
			memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			if (Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2 && (bufferCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
				memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			}

		VkMemoryAllocateInfo allocInfo = {};
			allocInfo.pNext = memoryAllocateFlagsInfo.flags > 0 ? &memoryAllocateFlagsInfo : nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = allocSize;
			VkMemoryPropertyFlags properties;
			switch (memoryUsage) {
				case MEMORY_USAGE_UNKNOWN: 
					properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
					break;
				case MEMORY_USAGE_GPU_ONLY: 
					properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
					break;
				case MEMORY_USAGE_CPU_ONLY: 
					properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
					break;
				case MEMORY_USAGE_CPU_TO_GPU: 
					properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
					break;
				case MEMORY_USAGE_GPU_TO_CPU: 
					properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
					break;
				case MEMORY_USAGE_CPU_COPY: 
					properties = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
					break;
				case MEMORY_USAGE_GPU_LAZILY_ALLOCATED: 
					properties = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
					break;
				case MEMORY_USAGE_MAX_ENUM: 
					properties = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
					break;
			}
			allocInfo.memoryTypeIndex = GetPhysicalDevice()->FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (AllocateMemory(&allocInfo, nullptr, pAllocation) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate buffer memory");
		}
		
		return BindBufferMemory(buffer, *pAllocation, 0);
	#endif
}
VkResult Device::CreateAndAllocateImage(const VkImageCreateInfo& imageCreateInfo, MemoryUsage memoryUsage, VkImage& image, MemoryAllocation* pAllocation, bool weakAllocation) {
	#ifdef V4D_VULKAN_USE_VMA
		VmaAllocationCreateInfo allocInfo {};
			allocInfo.usage = (VmaMemoryUsage)memoryUsage;
			if (weakAllocation && memoryUsage == MEMORY_USAGE_GPU_ONLY) {
				allocInfo.flags |= VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT | VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
			}
			if (Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2 && (imageCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
				allocInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			}
		return vmaCreateImage(allocator, &imageCreateInfo, &allocInfo, &image, pAllocation, nullptr);
	#else
		if ((CreateImage(&imageCreateInfo, nullptr, &image)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image");
		}

		VkMemoryRequirements memRequirements;
		GetImageMemoryRequirements(image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = GetPhysicalDevice()->FindMemoryType(memRequirements.memoryTypeBits, memoryUsage);
		if ((AllocateMemory(&allocInfo, nullptr, pAllocation)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory");
		}

		return BindImageMemory(image, *pAllocation, 0);
	#endif
}
void Device::FreeAndDestroyBuffer(VkBuffer& buffer, MemoryAllocation& allocation) {
	#ifdef V4D_VULKAN_USE_VMA
		std::lock_guard lock(allocatorDeleteMutex);
		if (allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
		// } else {
			// LOG_DEBUG("FreeAndDestroyBuffer called after allocator has been destroyed")
		}
	#else
		DestroyBuffer(buffer, nullptr);
		if (allocation) FreeMemory(allocation, nullptr);
	#endif
	buffer = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
}
void Device::FreeAndDestroyImage(VkImage& image, MemoryAllocation& allocation) {
	#ifdef V4D_VULKAN_USE_VMA
		std::lock_guard lock(allocatorDeleteMutex);
		if (allocator) {
			vmaDestroyImage(allocator, image, allocation);
		// } else {
			// LOG_DEBUG("FreeAndDestroyImage called after allocator has been destroyed")
		}
	#else
		DestroyImage(image, nullptr);
		if (allocation) FreeMemory(allocation, nullptr);
	#endif
	image = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
}
VkResult Device::MapMemoryAllocation(MemoryAllocation& allocation, void** data, VkDeviceSize offset, VkDeviceSize size) {
	#ifdef V4D_VULKAN_USE_VMA
		VkResult result = vmaMapMemory(allocator, allocation, data);
		if (result != VK_SUCCESS && offset > 0) {
			(*((uint8_t**)data)) += offset;
		}
		return result;
	#else
		return MapMemory(allocation, offset, size, 0, data);
	#endif
}
void Device::UnmapMemoryAllocation(MemoryAllocation& allocation) {
	#ifdef V4D_VULKAN_USE_VMA
		std::lock_guard lock(allocatorDeleteMutex);
		if (allocator) {
			vmaUnmapMemory(allocator, allocation);
		}
	#else
		UnmapMemory(allocation);
	#endif
}
VkResult Device::FlushMemoryAllocation(MemoryAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) {
	#ifdef V4D_VULKAN_USE_VMA
		return vmaFlushAllocation(allocator, allocation, offset, size);
	#else
		VkMappedMemoryRange mappedRange {};{
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = allocation;
			mappedRange.offset = offset;
			mappedRange.size = size;
		}
		return FlushMappedMemoryRanges(1, &mappedRange);
	#endif
}
void Device::AllocatorSetCurrentFrameIndex(uint32_t frameIndex) {
	#ifdef V4D_VULKAN_USE_VMA
		vmaSetCurrentFrameIndex(allocator, frameIndex);
	#endif
}
bool Device::TouchAllocation(MemoryAllocation& allocation) {
	#ifdef V4D_VULKAN_USE_VMA
		std::lock_guard lock(allocatorDeleteMutex);
		if (!allocator || !allocation) return false;
		return vmaTouchAllocation(allocator, allocation) == VK_TRUE;
	#endif
	return true;
}
