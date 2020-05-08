#include <v4d.h>

using namespace v4d::graphics;

#pragma region Configuration Methods

VkPhysicalDeviceVulkan12Features* Renderer::EnableVulkan12DeviceFeatures() {
	vulkan12DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	return &vulkan12DeviceFeatures;
}
VkPhysicalDeviceRayTracingFeaturesKHR* Renderer::EnableRayTracingFeatures() {
	rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
	return &rayTracingFeatures;
}

void Renderer::RequiredDeviceExtension(const char* ext) {
	requiredDeviceExtensions.push_back(std::move(ext));
}

void Renderer::OptionalDeviceExtension(const char* ext) {
	optionalDeviceExtensions.push_back(std::move(ext));
}

bool Renderer::IsDeviceExtensionEnabled(const char* ext) {
	return enabledDeviceExtensions.find(ext) != enabledDeviceExtensions.end();
}

#pragma endregion

#pragma region Virtual INIT Methods

void Renderer::CreateDevices() {
	
	// Select The Best Main PhysicalDevice using a score system
	renderingPhysicalDevice = SelectSuitablePhysicalDevice([this](int& score, PhysicalDevice* physicalDevice){
		// Build up a score here and the PhysicalDevice with the highest score will be selected.

		// Mandatory physicalDevice requirements for rendering graphics
		if (!physicalDevice->QueueFamiliesContainsFlags(VK_QUEUE_GRAPHICS_BIT, 1, &surface))
			return;
			
		// User-defined required extensions
		for (auto& ext : requiredDeviceExtensions) if (!physicalDevice->SupportsExtension(ext))
			return;
		
		score = 1;
		
		// Each Optional extensions adds one point to the score
		for (auto& ext : optionalDeviceExtensions) if (physicalDevice->SupportsExtension(ext))
			++score;

		// Modules
		V4D_Renderer::ForEachModule([&score, physicalDevice](auto* mod){
			if (mod->ScorePhysicalDeviceSelection) mod->ScorePhysicalDeviceSelection(score, physicalDevice);
		});
		
	});

	LOG("Selected Rendering PhysicalDevice: " << renderingPhysicalDevice->GetDescription());

	// Prepare enabled extensions
	deviceExtensions.clear();
	for (auto& ext : requiredDeviceExtensions) {
		deviceExtensions.push_back(ext);
		enabledDeviceExtensions[ext] = true;
		LOG("Enabling Device Extension: " << ext)
	}
	for (auto& ext : optionalDeviceExtensions) {
		if (renderingPhysicalDevice->SupportsExtension(ext)) {
			deviceExtensions.push_back(ext);
			enabledDeviceExtensions[ext] = true;
			LOG("Enabling Device Extension: " << ext)
		} else {
			enabledDeviceExtensions[ext] = false;
		}
	}
	
	// Prepare Device Features (and disable unsupported features)
	
	InitDeviceFeatures();
	
	FilterSupportedDeviceFeatures(&deviceFeatures, renderingPhysicalDevice->GetFeatures());
	void* pNext = nullptr;
	if (
		rayTracingFeatures.sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR
	 	&& renderingPhysicalDevice->SupportsExtension(VK_KHR_RAY_TRACING_EXTENSION_NAME)
	 	&& IsDeviceExtensionEnabled(VK_KHR_RAY_TRACING_EXTENSION_NAME)
	) {
		FilterSupportedDeviceFeatures(&rayTracingFeatures, renderingPhysicalDevice->GetRayTracingFeatures(), sizeof(VkStructureType)+sizeof(void*));
		EnableVulkan12DeviceFeatures()->pNext = &rayTracingFeatures; //TODO improve feature chains structure for more flexibility
	} else {
		rayTracingFeatures = {};
	}
	if (vulkan12DeviceFeatures.sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) {
		FilterSupportedDeviceFeatures(&vulkan12DeviceFeatures, renderingPhysicalDevice->GetVulkan12Features(), sizeof(VkStructureType)+sizeof(void*));
		pNext = &vulkan12DeviceFeatures;
	}
	
	// Create Logical Device
	renderingDevice = new Device(
		renderingPhysicalDevice,
		deviceFeatures,
		deviceExtensions,
		vulkanLoader->requiredInstanceLayers,
		queuesInfo,
		pNext
	);

	if (!renderingDevice->GetQueue("present").handle) {
		throw std::runtime_error("Failed to get Presentation Queue for surface. At least one queue must be defined with a VkSurfaceKHR or you must manually specify a 'present' queue");
	}
}

void Renderer::DestroyDevices() {
	
	delete renderingDevice;
}

void Renderer::CreateSyncObjects() {
	
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->CreateSyncObjects) mod->CreateSyncObjects();
	});
}

void Renderer::DestroySyncObjects() {
	
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->DestroySyncObjects) mod->DestroySyncObjects();
	});
}

void Renderer::CreateCommandPools() {
	
	// V4D_Renderer::ForEachSortedModule([this](auto* mod){
	// 	if (mod->CreateCommandPools) mod->CreateCommandPools();
	// });
	
	for (auto&[k, qs] : renderingDevice->GetQueues()) if (k != "present") {
		for (auto& q : qs) {
			if (!q.commandPool) {
				renderingDevice->CreateCommandPool(q.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &q.commandPool);
			}
		}
	}
}

void Renderer::DestroyCommandPools() {
	
	// V4D_Renderer::ForEachSortedModule([this](auto* mod){
	// 	if (mod->DestroyCommandPools) mod->DestroyCommandPools();
	// });
	
	for (auto&[k, qs] : renderingDevice->GetQueues()) if (k != "present") {
		for (auto& q : qs) {
			if (q.commandPool) {
				renderingDevice->DestroyCommandPool(q.commandPool);
			}
		}
	}
}

void Renderer::CreateDescriptorSets() {
	
	for (auto[name,set] : descriptorSets) {
		set->CreateDescriptorSetLayout(renderingDevice);
	}
	
	// Descriptor sets / pool
	std::map<VkDescriptorType, uint> descriptorTypes {};
	for (auto[name,set] : descriptorSets) {
		for (auto&[binding, descriptor] : set->GetBindings()) {
			if (descriptorTypes.find(descriptor.descriptorType) == descriptorTypes.end()) {
				descriptorTypes[descriptor.descriptorType] = 1;
			} else {
				descriptorTypes[descriptor.descriptorType]++;
			}
		}
	}
	
	if (descriptorSets.size() > 0) {
		renderingDevice->CreateDescriptorPool(
			descriptorTypes,
			descriptorPool,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
		);
		
		// Allocate descriptor sets
		std::vector<VkDescriptorSetLayout> setLayouts {};
		vkDescriptorSets.resize(descriptorSets.size());
		setLayouts.reserve(descriptorSets.size());
		for (auto[name,set] : descriptorSets) {
			setLayouts.push_back(set->GetDescriptorSetLayout());
		}
		VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = (uint)setLayouts.size();
			allocInfo.pSetLayouts = setLayouts.data();
		if (renderingDevice->AllocateDescriptorSets(&allocInfo, vkDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets");
		}
		int i = 0;
		for (auto[name,set] : descriptorSets) {
			descriptorSets[name]->descriptorSet = vkDescriptorSets[i++];
		}
	}
	
	UpdateDescriptorSets();
}

void Renderer::DestroyDescriptorSets() {
	
	// Descriptor Sets
	if (descriptorSets.size() > 0) {
		renderingDevice->FreeDescriptorSets(descriptorPool, (uint)vkDescriptorSets.size(), vkDescriptorSets.data());
		for (auto[name,set] : descriptorSets) set->DestroyDescriptorSetLayout(renderingDevice);
		// Descriptor pools
		renderingDevice->DestroyDescriptorPool(descriptorPool, nullptr);
	}
}

void Renderer::UpdateDescriptorSets() {
	
	if (descriptorSets.size() > 0) {
		std::vector<VkWriteDescriptorSet> descriptorWrites {};
		for (auto[name,set] : descriptorSets) {
			for (auto&[binding, descriptor] : set->GetBindings()) {
				if (descriptor.IsWriteDescriptorSetValid()) {
					descriptorWrites.push_back(descriptor.GetWriteDescriptorSet(set->descriptorSet));
				}
			}
		}
		renderingDevice->UpdateDescriptorSets((uint)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
}

void Renderer::UpdateDescriptorSet(DescriptorSet* set, const std::vector<uint32_t>& bindings) {
	std::vector<VkWriteDescriptorSet> descriptorWrites {};
	for (auto&[binding, descriptor] : set->GetBindings()) if (bindings.size() == 0 || std::find(bindings.begin(), bindings.end(), binding) != bindings.end()) {
		if (descriptor.IsWriteDescriptorSetValid()) {
			descriptorWrites.push_back(descriptor.GetWriteDescriptorSet(set->descriptorSet));
		}
	}
	renderingDevice->UpdateDescriptorSets((uint)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

bool Renderer::CreateSwapChain() {
	
	// Put old swapchain in a temporary pointer and delete it after creating new swapchain
	SwapChain* oldSwapChain = swapChain;

	// Create the new swapchain object
	swapChain = new SwapChain(
		renderingDevice,
		surface,
		NB_FRAMES_IN_FLIGHT,
		{ // Preferred Extent (Screen Resolution)
			0, // width
			0 // height
		},
		preferredFormats,
		preferredPresentModes
	);
	// Assign queues
	// swapChain->AssignQueues({renderingDevice->GetQueue("present").familyIndex, renderingDevice->GetQueue("graphics").familyIndex});
	// Set custom params
	// swapChain->createInfo.xxxx = xxxxxx...
	// swapChain->imageViewsCreateInfo.xxxx = xxxxxx...
	
	if (swapChain->extent.width == 0 || swapChain->extent.height == 0) {
		return false;
	}

	// Create the swapchain handle and corresponding imageviews
	swapChain->Create(oldSwapChain);

	// Destroy the old swapchain
	if (oldSwapChain != nullptr) {
		oldSwapChain->Destroy();
		delete oldSwapChain;
	}
	
	return true;
}

void Renderer::DestroySwapChain() {
	swapChain->Destroy();
	delete swapChain;
	swapChain = nullptr;
}

void Renderer::CreateCommandBuffers() {
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->CreateCommandBuffers) mod->CreateCommandBuffers();
	});
}

void Renderer::DestroyCommandBuffers() {
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->DestroyCommandBuffers) mod->DestroyCommandBuffers();
	});
}

#pragma endregion

#pragma region Helper methods

VkCommandBuffer Renderer::BeginSingleTimeCommands(const Queue& queue) {
	return renderingDevice->BeginSingleTimeCommands(queue);
}

void Renderer::EndSingleTimeCommands(const Queue& queue, VkCommandBuffer commandBuffer) {
	renderingDevice->EndSingleTimeCommands(queue, commandBuffer);
}

void Renderer::RunSingleTimeCommands(const Queue& queue, std::function<void(VkCommandBuffer)>&& func) {
	renderingDevice->RunSingleTimeCommands(queue, std::forward<std::function<void(VkCommandBuffer)>>(func));
}

void Renderer::AllocateBufferStaged(const Queue& queue, Buffer& buffer) {
	auto cmdBuffer = BeginSingleTimeCommands(queue);
	Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	stagingBuffer.srcDataPointers = std::ref(buffer.srcDataPointers);
	stagingBuffer.Allocate(renderingDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
	buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer.Allocate(renderingDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
	Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer.buffer, buffer.size);
	EndSingleTimeCommands(queue, cmdBuffer);
	stagingBuffer.Free(renderingDevice);
}
void Renderer::AllocateBuffersStaged(const Queue& queue, std::vector<Buffer>& buffers) {
	auto cmdBuffer = BeginSingleTimeCommands(queue);
	std::vector<Buffer> stagingBuffers {};
	stagingBuffers.reserve(buffers.size());
	for (auto& buffer : buffers) {
		auto& stagingBuffer = stagingBuffers.emplace_back(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingBuffer.srcDataPointers = std::ref(buffer.srcDataPointers);
		stagingBuffer.Allocate(renderingDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
		buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer.Allocate(renderingDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
		Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer.buffer, buffer.size);
	}
	EndSingleTimeCommands(queue, cmdBuffer);
	for (auto& stagingBuffer : stagingBuffers) {
		stagingBuffer.Free(renderingDevice);
	}
}
void Renderer::AllocateBuffersStaged(const Queue& queue, std::vector<Buffer*>& buffers) {
	auto cmdBuffer = BeginSingleTimeCommands(queue);
	std::vector<Buffer> stagingBuffers {};
	stagingBuffers.reserve(buffers.size());
	for (auto& buffer : buffers) {
		auto& stagingBuffer = stagingBuffers.emplace_back(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingBuffer.srcDataPointers = std::ref(buffer->srcDataPointers);
		stagingBuffer.Allocate(renderingDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
		buffer->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer->Allocate(renderingDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
		Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer->buffer, buffer->size);
	}
	EndSingleTimeCommands(queue, cmdBuffer);
	for (auto& stagingBuffer : stagingBuffers) {
		stagingBuffer.Free(renderingDevice);
	}
}

// void Renderer::AllocateBufferStaged(Buffer& buffer) {
// 	AllocateBufferStaged(renderingDevice->GetQueue("..."), buffer);
// }
// void Renderer::AllocateBuffersStaged(std::vector<Buffer>& buffers) {
// 	AllocateBuffersStaged(renderingDevice->GetQueue("..."), buffers);
// }
// void Renderer::AllocateBuffersStaged(std::vector<Buffer*>& buffers) {
// 	AllocateBuffersStaged(renderingDevice->GetQueue("..."), buffers);
// }

// void Renderer::TransitionImageLayout(Image image, VkImageLayout oldLayout, VkImageLayout newLayout) {
// 	auto commandBuffer = BeginSingleTimeCommands(renderingDevice->GetQueue("..."));
// 	TransitionImageLayout(commandBuffer, image, oldLayout, newLayout);
// 	EndSingleTimeCommands(renderingDevice->GetQueue("..."), commandBuffer);
// }
void Renderer::TransitionImageLayout(VkCommandBuffer commandBuffer, Image image, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkImageAspectFlags aspectMask = 0;
	if (image.format == VK_FORMAT_D32_SFLOAT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (image.format == VK_FORMAT_D32_SFLOAT_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if (!aspectMask && (image.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	TransitionImageLayout(commandBuffer, image.image, oldLayout, newLayout, image.mipLevels, image.arrayLayers, aspectMask);
}
// void Renderer::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, VkImageAspectFlags aspectMask) {
// 	auto commandBuffer = BeginSingleTimeCommands(renderingDevice->GetQueue("..."));
// 	TransitionImageLayout(commandBuffer, image, oldLayout, newLayout, mipLevels, layerCount, aspectMask);
// 	EndSingleTimeCommands(renderingDevice->GetQueue("..."), commandBuffer);
// }
void Renderer::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, VkImageAspectFlags aspectMask) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout; // VK_IMAGE_LAYOUT_UNDEFINED if we dont care about existing contents of the image
	barrier.newLayout = newLayout;
	// If we are using the barrier to transfer queue family ownership, these two fields should be the indices of the queue families. Otherwise VK_QUEUE_FAMILY_IGNORED
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	//
	if (aspectMask) {
		barrier.subresourceRange.aspectMask = aspectMask;
	} else {
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		} else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	//
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	
	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source 
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	/*
	Transfer writes must occur in the pipeline transfer stage. 
	Since the writes dont have to wait on anything, we mayy specify an empty access mask and the earliest possible pipeline stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT for the pre-barrier operations.
	It should be noted that VK_PIPELINE_STAGE_TRANSFER_BIT is not a Real stage within the graphics and compute pipelines.
	It is more of a pseudo-stage where transfers happen.
	
	The image will be written in the same pipeline stage and subsequently read by the fragment shader, which is why we specify shader reading access in the fragment pipeline stage.
	If we need to do more transitions in the future, then we'll extend the function.
	
	One thing to note is that command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT synchronization at the beginning.
	Since the TransitionImageLayout function executes a command buffer with only a single command, we could use this implicit synchronization and set srcAccessMask to 0 if we ever needed a VK_ACCESS_HOST_WRITE_BIT dependency in a layout transition.

	There is actually a special type of image layout that supports all operations, VK_IMAGE_LAYOUT_GENERAL.
	The problem with it is that it doesnt necessarily offer the best performance for any operation.
	It is required for some special cases, like using an image as both input and output, or for reading an image after it has left the preinitialized layout.
	*/
	//
	renderingDevice->CmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

// void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
// 	auto commandBuffer = BeginSingleTimeCommands(renderingDevice->GetQueue("..."));
// 	CopyBufferToImage(commandBuffer, buffer, image, width, height);
// 	EndSingleTimeCommands(renderingDevice->GetQueue("..."), commandBuffer);
// }
// void Renderer::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
// 	VkBufferImageCopy region = {};
// 	region.bufferOffset = 0;
// 	region.bufferRowLength = 0;
// 	region.bufferImageHeight = 0;
	
// 	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	region.imageSubresource.mipLevel = 0;
// 	region.imageSubresource.baseArrayLayer = 0;
// 	region.imageSubresource.layerCount = 1;

// 	region.imageOffset = {0, 0, 0};
// 	region.imageExtent = {width, height, 1};

// 	renderingDevice->CmdCopyBufferToImage(
// 		commandBuffer,
// 		buffer,
// 		image,
// 		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 		1,
// 		&region
// 	);
// }

// void Renderer::GenerateMipmaps(Texture2D* texture) {
// 	GenerateMipmaps(texture->GetImage(), texture->GetFormat(), texture->GetWidth(), texture->GetHeight(), texture->GetMipLevels());
// }

// void Renderer::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels) {
// 	VkFormatProperties formatProperties;
// 	renderingPhysicalDevice->GetPhysicalDeviceFormatProperties(imageFormat, &formatProperties);
// 	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
// 		throw std::runtime_error("Texture image format does not support linear blitting");
// 	}

// 	auto commandBuffer = BeginSingleTimeCommands(renderingDevice->GetQueue("..."));

// 	VkImageMemoryBarrier barrier = {};
// 		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 		barrier.image = image;
// 		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		barrier.subresourceRange.baseArrayLayer = 0;
// 		barrier.subresourceRange.layerCount = 1;
// 		barrier.subresourceRange.levelCount = 1;

// 	int32_t mipWidth = width;
// 	int32_t mipHeight = height;

// 	for (uint32_t i = 1; i < mipLevels; i++) {
// 		barrier.subresourceRange.baseMipLevel = i - 1;
// 		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

// 		renderingDevice->CmdPipelineBarrier(
// 			commandBuffer, 
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier
// 		);

// 		VkImageBlit blit = {};
// 			blit.srcOffsets[0] = { 0, 0, 0 };
// 			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
// 			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			blit.srcSubresource.mipLevel = i - 1;
// 			blit.srcSubresource.baseArrayLayer = 0;
// 			blit.srcSubresource.layerCount = 1;
// 			blit.dstOffsets[0] = { 0, 0, 0 };
// 			blit.dstOffsets[1] = { mipWidth>1? mipWidth/2 : 1, mipHeight>1? mipHeight/2 : 1, 1 };
// 			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			blit.dstSubresource.mipLevel = i;
// 			blit.dstSubresource.baseArrayLayer = 0;
// 			blit.dstSubresource.layerCount = 1;

// 		renderingDevice->CmdBlitImage(
// 			commandBuffer,
// 			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
// 			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 			1, &blit,
// 			VK_FILTER_LINEAR
// 		);

// 		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 		renderingDevice->CmdPipelineBarrier(
// 			commandBuffer,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier
// 		);

// 		if (mipWidth > 1) mipWidth /= 2;
// 		if (mipHeight > 1) mipHeight /= 2;
// 	}

// 	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
// 	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 	renderingDevice->CmdPipelineBarrier(
// 		commandBuffer,
// 		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 		0, nullptr,
// 		0, nullptr,
// 		1, &barrier
// 	);

// 	EndSingleTimeCommands(renderingDevice->GetQueue("..."), commandBuffer);
// }

#pragma endregion

#pragma region Init/Load/Reset Methods

void Renderer::RecreateSwapChains() {
	std::scoped_lock lock(renderMutex1, renderMutex2);
	
	v4d::graphics::renderer::event::Unload(this);
	
	if (graphicsLoadedToDevice)
		UnloadGraphicsFromDevice();
		
	v4d::graphics::renderer::event::Resize(this);
	
	// Re-Create the SwapChain
	if (!CreateSwapChain()) {
		SLEEP(100ms)
		return;
	}
	
	LoadGraphicsToDevice();
	v4d::graphics::renderer::event::Load(this);
}

void Renderer::InitRenderer() {
	Init();
	InitLayouts();
	ConfigureShaders();
}

void Renderer::LoadRenderer() {
	CreateDevices();
	ConfigureRenderer();
	CreateSyncObjects();
	
	if (!CreateSwapChain()) {
		SLEEP(100ms)
		return;
	}
	
	LoadGraphicsToDevice();
	v4d::graphics::renderer::event::Load(this);
}

void Renderer::UnloadRenderer() {
	v4d::graphics::renderer::event::Unload(this);
	
	if (graphicsLoadedToDevice)
		UnloadGraphicsFromDevice();
	
	DestroySwapChain();
	DestroySyncObjects();
	DestroyDevices();
}

void Renderer::ReloadRenderer() {
	std::scoped_lock lock(renderMutex1, renderMutex2);
	
	if (renderThreadId != std::this_thread::get_id()) {
		mustReload = true;
		return;
	}
	
	mustReload = false;
	
	LOG("Reloading renderer...")
	v4d::graphics::renderer::event::Unload(this);
	
	if (graphicsLoadedToDevice)
		UnloadGraphicsFromDevice();
	
	DestroySwapChain();
	DestroySyncObjects();
	DestroyDevices();
	
	v4d::graphics::renderer::event::Reload(this);
	
	ReadShaders();
	
	CreateDevices();
	ConfigureRenderer();
	CreateSyncObjects();
	
	if (!CreateSwapChain()) {
		SLEEP(100ms)
		return;
	}
	
	LoadGraphicsToDevice();
	v4d::graphics::renderer::event::Load(this);
}

void Renderer::LoadGraphicsToDevice() {
	CreateCommandPools();
	AllocateBuffers();
	CreateResources();
	CreateDescriptorSets();
	CreatePipelines(); // shaders are assigned here
	
	v4d::graphics::renderer::event::PipelinesCreate(this);
	
	CreateCommandBuffers(); // objects are rendered here
	
	graphicsLoadedToDevice = true;
	LOG_SUCCESS("Vulkan Renderer is Ready !")
}

void Renderer::UnloadGraphicsFromDevice() {
	// Wait for renderingDevice to be idle before destroying everything
	renderingDevice->DeviceWaitIdle(); // We can also wait for operations in a specific command queue to be finished with vkQueueWaitIdle. These functions can be used as a very rudimentary way to perform synchronization. 

	DestroyCommandBuffers();
	
	v4d::graphics::renderer::event::PipelinesDestroy(this);
	
	DestroyPipelines();
	DestroyDescriptorSets();
	DestroyResources();
	FreeBuffers();
	DestroyCommandPools();
	
	graphicsLoadedToDevice = false;
}

#pragma endregion

#pragma region Constructor & Destructor

Renderer::Renderer(Loader* loader, const char* applicationName, uint applicationVersion)
: Instance(loader, applicationName, applicationVersion, true) {}

Renderer::~Renderer() {}

#pragma endregion

#pragma region V4D_Renderer modules passthrough

// Init
void Renderer::Init() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->Init) mod->Init(this);
	});
}

void Renderer::InitDeviceFeatures() {
	// Modules
	V4D_Renderer::ForEachSortedModule([](auto* mod){
		if (mod->InitDeviceFeatures) mod->InitDeviceFeatures();
	});
}

void Renderer::ConfigureRenderer() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->ConfigureRenderer) mod->ConfigureRenderer();
	});
}

void Renderer::InitLayouts() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->InitLayouts) mod->InitLayouts();
	});
}

void Renderer::ConfigureShaders() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->ConfigureShaders) mod->ConfigureShaders();
	});
}

// Scene
void Renderer::ReadShaders() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->ReadShaders) mod->ReadShaders();
	});
}

void Renderer::LoadScene() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->LoadScene) mod->LoadScene();
	});
}

void Renderer::UnloadScene() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->UnloadScene) mod->UnloadScene();
	});
}

// Resources
void Renderer::CreateResources() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->CreateResources) mod->CreateResources();
	});
}

void Renderer::DestroyResources() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->DestroyResources) mod->DestroyResources();
	});
}

void Renderer::AllocateBuffers() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->AllocateBuffers) mod->AllocateBuffers();
	});
}

void Renderer::FreeBuffers() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->FreeBuffers) mod->FreeBuffers();
	});
}

// Pipelines
void Renderer::CreatePipelines() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->CreatePipelines) mod->CreatePipelines();
	});
}

void Renderer::DestroyPipelines() {
	// Modules
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->DestroyPipelines) mod->DestroyPipelines();
	});
}

void Renderer::Render() {
	renderThreadId = std::this_thread::get_id();
	std::scoped_lock lock(renderMutex1);
	
	if (!graphicsLoadedToDevice) {
		RecreateSwapChains();
		return;
	}
	
	if (mustReload) {
		ReloadRenderer();
		return;
	}
	
	V4D_Renderer::ForEachSortedModule([this](auto* mod){
		if (mod->Render) mod->Render();
	});
}

#pragma endregion
