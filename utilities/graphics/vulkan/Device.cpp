#include "Device.h"
#include "Instance.h"
#include "Buffer.h"

using namespace v4d::graphics::vulkan;

Device::Device(
	PhysicalDevice* physicalDevice,
	std::vector<const char*>& extensions,
	std::vector<const char*>& layers,
	std::map<VkQueueFlags, std::map<uint, Queue>>& queues,
	void* pNext
) : physicalDevice(physicalDevice), queues(queues) {
	instance = physicalDevice->GetVulkanInstance();

	// Queues
	std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo {};
	std::map<int, std::vector<float>> queuesCreateInfoPriorities {};
	for (auto& [key, qs] : queues) if (qs.size() > 0) {
		bool foundQueueFamilyIndex = false;
		for (auto queueFamilyIndex : physicalDevice->GetQueueFamilyIndicesFromFlags(qs[0].flags, qs.size(), qs[0].surface)) {
			auto existing = std::find_if(queuesCreateInfo.begin(), queuesCreateInfo.end(), [queueFamilyIndex](VkDeviceQueueCreateInfo& n){return n.queueFamilyIndex == (uint32_t)queueFamilyIndex;});
			// If queueFamilyIndex is NOT already present in our queuesCreateInfo vector, add it
			if (existing == queuesCreateInfo.end()) {
				size_t createInfoIndex = queuesCreateInfo.size();
				for (auto& [i, q] : qs) {
					q.family = queueFamilyIndex;
					q.index = i;
					queuesCreateInfoPriorities[createInfoIndex].push_back(q.priority);
				}
				queuesCreateInfo.emplace_back(VkDeviceQueueCreateInfo{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType sType
					nullptr, // const void* pNext
					0, // VkDeviceQueueCreateFlags flags
					(uint32_t)queueFamilyIndex, // uint32_t queueFamilyIndex
					(uint32_t)qs.size(), // uint32_t queueCount
					queuesCreateInfoPriorities[createInfoIndex].data() // const float* pQueuePriorities
				});
				foundQueueFamilyIndex = true;
				break;
			}
		}
		if (!foundQueueFamilyIndex) {
			// If could not find a different queue family, try to use an existing one
			size_t createInfoIndex = 0;
			for (auto& existingQueuesCreateInfo : queuesCreateInfo) {
				if (existingQueuesCreateInfo.queueFamilyIndex == (uint32_t)physicalDevice->GetQueueFamilyIndexFromFlags(qs[0].flags, qs.size() + existingQueuesCreateInfo.queueCount, qs[0].surface)) {
					for (auto& [i, q] : qs) {
						q.family = existingQueuesCreateInfo.queueFamilyIndex;
						q.index = i + existingQueuesCreateInfo.queueCount;
						queuesCreateInfoPriorities[createInfoIndex].push_back(q.priority);
						existingQueuesCreateInfo.queueCount++;
					}
					existingQueuesCreateInfo.pQueuePriorities = queuesCreateInfoPriorities[createInfoIndex].data();
					foundQueueFamilyIndex = true;
					break;
				}
				createInfoIndex++;
			}
		}
		if (!foundQueueFamilyIndex) {
			throw std::runtime_error("Failed to find a suitable queue for " + std::to_string(int(key)) + " with flags " + std::to_string(int(qs[0].flags)) + " and queueCount " + std::to_string(int(qs.size())));
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

	Instance::CheckVkResult("Failed to create logical device", physicalDevice->CreateDevice(&createInfo, nullptr, &handle));
	
	LoadFunctionPointers();

	// Get Queues Handles
	for (auto& [key, qs] : queues) {
		for (auto& [i, q] : qs) {
			GetDeviceQueue(q.family, q.index, &q.handle);
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

VkCommandBuffer Device::BeginSingleTimeCommands(Queue queue, uint commandPoolIndex) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = queue.commandPools[commandPoolIndex];
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	Instance::CheckVkResult("BeginSingleTimeCommands::AllocateCommandBuffers", AllocateCommandBuffers(&allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	Instance::CheckVkResult("BeginSingleTimeCommands::BeginCommandBuffer", BeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void Device::EndSingleTimeCommands(Queue queue, VkCommandBuffer commandBuffer, uint commandPoolIndex) {
	Instance::CheckVkResult("EndSingleTimeCommands::EndCommandBuffer", EndCommandBuffer(commandBuffer));

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
	
	FreeCommandBuffers(queue.commandPools[commandPoolIndex], 1, &commandBuffer);
	
	// QueueWaitIdle(queue.handle);
}

void Device::RunSingleTimeCommands(Queue queue, std::function<void(VkCommandBuffer)>&& func, uint commandPoolIndex) {
	auto cmdBuffer = BeginSingleTimeCommands(queue, commandPoolIndex);
	func(cmdBuffer);
	EndSingleTimeCommands(queue, cmdBuffer, commandPoolIndex);
}

bool Device::TryRunSingleTimeCommands(Queue queue, std::function<bool(VkCommandBuffer)>&& func, uint commandPoolIndex) {
	auto cmdBuffer = BeginSingleTimeCommands(queue, commandPoolIndex);
	bool result = func(cmdBuffer);
	if (result) {
		EndSingleTimeCommands(queue, cmdBuffer, commandPoolIndex);
	} else {
		FreeCommandBuffers(queue.commandPools[commandPoolIndex], 1, &cmdBuffer);
	}
	return result;
}

void Device::DumpMemoryAllocationStats() {
	char* str;
	vmaBuildStatsString(allocator, &str, true);
	std::ofstream file("vramdump.json");
	file << str << std::endl;
	vmaFreeStatsString(allocator, str);
}

// Allocator
void Device::CreateAllocator() {
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
			allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}
	}
	vmaCreateAllocator(&allocatorInfo, &allocator);
}
void Device::DestroyAllocator() {
	vmaDestroyAllocator(allocator);
	allocator = VK_NULL_HANDLE;
}
VkResult Device::CreateAndAllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VmaPool pool, VkBuffer& buffer, MemoryAllocation* pAllocation) {
	VmaAllocationCreateInfo allocInfo {};
		allocInfo.pool = pool;
	return vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &buffer, pAllocation, nullptr);
}
VkResult Device::CreateAndAllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, MemoryUsage memoryUsage, VkBuffer& buffer, MemoryAllocation* pAllocation, bool weakAllocation) {
	VmaAllocationCreateInfo allocInfo {};
		allocInfo.usage = (VmaMemoryUsage)memoryUsage;
		if (weakAllocation && memoryUsage == MEMORY_USAGE_GPU_ONLY) {
			allocInfo.flags |= VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT | VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
		}
	return vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &buffer, pAllocation, nullptr);
}
VkResult Device::CreateAndAllocateImage(const VkImageCreateInfo& imageCreateInfo, MemoryUsage memoryUsage, VkImage& image, MemoryAllocation* pAllocation, bool weakAllocation) {
	VmaAllocationCreateInfo allocInfo {};
		allocInfo.usage = (VmaMemoryUsage)memoryUsage;
		if (weakAllocation && memoryUsage == MEMORY_USAGE_GPU_ONLY) {
			allocInfo.flags |= VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT | VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
		}
		if (Loader::VULKAN_API_VERSION >= VK_API_VERSION_1_2 && (imageCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
			allocInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}
	return vmaCreateImage(allocator, &imageCreateInfo, &allocInfo, &image, pAllocation, nullptr);
}
void Device::FreeAndDestroyBuffer(VkBuffer& buffer, MemoryAllocation& allocation, uint64_t frameIndex) {
	assert(allocator);
	// if (frameIndex == 0 || frameIndex + 1 < deferredDeallocationCurrentFrame) {
		vmaDestroyBuffer(allocator, buffer, allocation);
	// } else {
	// 	DeferDeallocation(buffer, allocation, frameIndex + 1);
	// }
	buffer = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
}
void Device::FreeAndDestroyImage(VkImage& image, MemoryAllocation& allocation, uint64_t frameIndex) {
	assert(allocator);
	// if (frameIndex == 0 || frameIndex + 1 < deferredDeallocationCurrentFrame) {
		vmaDestroyImage(allocator, image, allocation);
	// } else {
	// 	DeferDeallocation(image, allocation, frameIndex + 1);
	// }
	image = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
}
VkResult Device::MapMemoryAllocation(MemoryAllocation& allocation, void** data, VkDeviceSize offset, VkDeviceSize size) {
	VkResult result = vmaMapMemory(allocator, allocation, data);
	if (result == VK_SUCCESS && offset > 0) {
		*((uint8_t**)data) += offset;
	}
	return result;
}
void Device::UnmapMemoryAllocation(MemoryAllocation& allocation) {
	assert(allocator);
	vmaUnmapMemory(allocator, allocation);
}
VkResult Device::FlushMemoryAllocation(MemoryAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) {
	return vmaFlushAllocation(allocator, allocation, offset, size);
}
void Device::AllocatorSetCurrentFrameIndex(uint32_t frameIndex) {
	vmaSetCurrentFrameIndex(allocator, frameIndex);
}
bool Device::TouchAllocation(MemoryAllocation& allocation) {
	if (!allocator || !allocation) return false;
	return vmaTouchAllocation(allocator, allocation) == VK_TRUE;
}

// uint64_t Device::deferredDeallocationCurrentFrame = 0;
// std::atomic<uint64_t> Device::nextDeferredDeallocationIndex = 0;
// std::array<Device::DeferredDeallocation, V4D_DEFERRED_DEALLOCATION_MAX_COUNT> Device::deferredDeallocations;
// Device::DeferredDeallocation* Device::GetNextDeferredDeallocation() {
// 	int nbTries = 0;
// 	uint64_t index;
// 	bool locked;
// 	do {
// 		assert(++nbTries < V4D_DEFERRED_DEALLOCATION_MAX_COUNT);
// 		index = nextDeferredDeallocationIndex++ % V4D_DEFERRED_DEALLOCATION_MAX_COUNT;
// 		locked = deferredDeallocations[index].lock.exchange(true);
// 	} while (locked);
// 	return &deferredDeallocations[index];
// }
// void Device::SetDeferredDeallocationCurrentFrame(uint64_t frameIndex) {
// 	Buffer::currentFrameIndex = frameIndex;
// 	deferredDeallocationCurrentFrame = frameIndex;
// }
// void Device::DestroyDeferredDeallocations(bool force) {
// 	for (size_t i = 0; i < V4D_DEFERRED_DEALLOCATION_MAX_COUNT; ++i) {
// 		auto& deferredDealloc = deferredDeallocations[i];
// 		if (deferredDealloc.allocation && (force || deferredDealloc.frameIndex < deferredDeallocationCurrentFrame)) {
// 			if (deferredDealloc.lock.exchange(true) == false || force) {
// 				if (deferredDealloc.allocation && (force || deferredDealloc.frameIndex < deferredDeallocationCurrentFrame)) {
// 					if (deferredDealloc.buffer) {
// 						vmaDestroyBuffer(allocator, deferredDealloc.buffer, deferredDealloc.allocation);
// 						deferredDealloc.buffer = 0;
// 					} else if (deferredDealloc.img) {
// 						vmaDestroyImage(allocator, deferredDealloc.img, deferredDealloc.allocation);
// 						deferredDealloc.img = 0;
// 					}
// 					deferredDealloc.allocation = 0;
// 				}
// 				deferredDealloc.lock = false;
// 			}
// 		}
// 	}
// }
