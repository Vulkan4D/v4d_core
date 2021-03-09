#include <v4d.h>

using namespace v4d::graphics;

#pragma region Static maps

	// Ray-Tracing Shaders
	std::unordered_map<std::string, uint32_t> Renderer::sbtOffsets {};

#pragma endregion

#pragma region Configuration Methods

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
		V4D_Mod::ForEachModule([&score, physicalDevice](auto mod){
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
	
	// Prepare Device Features
	PhysicalDevice::DeviceFeatures enabledDeviceFeatures {};
	InitDeviceFeatures(&enabledDeviceFeatures, &renderingPhysicalDevice->deviceFeatures);
	renderingPhysicalDevice->deviceFeatures &= enabledDeviceFeatures;
	
	// Create Logical Device
	renderingDevice = new Device(
		renderingPhysicalDevice,
		deviceExtensions,
		vulkanLoader->requiredInstanceLayers,
		queuesInfo,
		renderingPhysicalDevice->deviceFeatures.GetDeviceFeaturesPNext()
	);

	if (!renderingDevice->GetQueue("present").handle) {
		throw std::runtime_error("Failed to get Presentation Queue for surface. At least one queue must be defined with a VkSurfaceKHR or you must manually specify a 'present' queue");
	}
	
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->InitRenderingDevice) mod->InitRenderingDevice(renderingDevice);
	});
	renderingDevice->CreateAllocator();
}

void Renderer::DestroyDevices() {
	renderingDevice->DeviceWaitIdle();
	renderingDevice->DestroyAllocator();
	delete renderingDevice;
}

void Renderer::CreateSyncObjects() {
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->CreateVulkanSyncObjects) mod->CreateVulkanSyncObjects();
	});
}

void Renderer::DestroySyncObjects() {
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->DestroyVulkanSyncObjects) mod->DestroyVulkanSyncObjects();
	});
}

void Renderer::CreateCommandPools() {
	for (auto&[k, qs] : renderingDevice->GetQueues()) if (k != "present") {
		for (auto& q : qs) {
			if (!q.commandPool) {
				renderingDevice->CreateCommandPool(q.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &q.commandPool);
			}
		}
	}
}

void Renderer::DestroyCommandPools() {
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
	swapChain = nullptr;

	do {
		if (swapChain) {
			delete swapChain;
			SLEEP(100ms)
		}
		
		// Create the new swapchain object
		swapChain = new SwapChain(
			renderingDevice,
			surface,
			2,
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
		
	} while (swapChain->extent.width == 0 || swapChain->extent.height == 0);

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
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->CreateVulkanCommandBuffers) mod->CreateVulkanCommandBuffers();
	});
}

void Renderer::DestroyCommandBuffers() {
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->DestroyVulkanCommandBuffers) mod->DestroyVulkanCommandBuffers();
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
	stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
	buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer.Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
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
		stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
		buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer.Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
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
		stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
		buffer->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buffer->Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
		Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer->buffer, buffer->size);
	}
	EndSingleTimeCommands(queue, cmdBuffer);
	for (auto& stagingBuffer : stagingBuffers) {
		stagingBuffer.Free(renderingDevice);
	}
}

void Renderer::TransitionImageLayout(VkCommandBuffer commandBuffer, Image image, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkImageAspectFlags aspectMask = 0;
	if (image.format == VK_FORMAT_D32_SFLOAT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (image.format == VK_FORMAT_D32_SFLOAT_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if (!aspectMask && (image.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	TransitionImageLayout(commandBuffer, image.image, oldLayout, newLayout, image.mipLevels, image.arrayLayers, aspectMask);
}
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

#pragma endregion

#pragma region Init/Load/Reset Methods

void Renderer::RecreateSwapChains() {
	ReloadRenderer();return; // Temporary fix for crash when resizing window
	
	// std::scoped_lock lock(renderMutex1, renderMutex2);
	
	// v4d::graphics::renderer::event::Unload(this);
	
	// if (graphicsLoadedToDevice)
	// 	UnloadGraphicsFromDevice();
		
	// v4d::graphics::renderer::event::Resize(this);
	
	// LoadGraphicsToDevice();
	// v4d::graphics::renderer::event::Load(this);
}

void Renderer::InitRenderer() {
	InitLayouts();
	ConfigureShaders();
}

void Renderer::LoadRenderer() {
	CreateDevices();
	ConfigureRenderer();
	CreateSyncObjects();
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
	if (renderThreadId != std::this_thread::get_id()) {
		mustReload = true;
		return;
	}
	std::scoped_lock lock(renderMutex1, renderMutex2);
	
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
	
	LoadGraphicsToDevice();
	v4d::graphics::renderer::event::Load(this);
}

void Renderer::LoadGraphicsToDevice() {
	std::scoped_lock lock(renderMutex1, renderMutex2);
	while (!CreateSwapChain()) {
		LOG_WARN("Failed to create SwapChain... Reloading renderer...")
		SLEEP(1s)
		ReloadRenderer();
		return;
	}
	
	CreateCommandPools();
	AllocateBuffers();
	CreateResources();
	CreateDescriptorSets();
	CreatePipelines(); // shaders are assigned here
	
	v4d::graphics::renderer::event::PipelinesCreate(this);
	
	CreateCommandBuffers(); // objects are rendered here
	
	frameIndex = 0;
	graphicsLoadedToDevice = true;
	LOG_SUCCESS("Vulkan Renderer is Ready !")
}

void Renderer::UnloadGraphicsFromDevice() {
	std::scoped_lock lock(renderMutex1, renderMutex2);
	
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
: Instance(loader, applicationName, applicationVersion) {}

Renderer::~Renderer() {}

#pragma endregion

#pragma region V4D_Renderer modules passthrough

void Renderer::InitDeviceFeatures(PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const PhysicalDevice::DeviceFeatures* supportedDeviceFeatures) {
	// Modules
	V4D_Mod::ForEachSortedModule([deviceFeaturesToEnable, supportedDeviceFeatures](auto mod){
		if (mod->InitVulkanDeviceFeatures) mod->InitVulkanDeviceFeatures(deviceFeaturesToEnable, supportedDeviceFeatures);
	});
}

void Renderer::ConfigureRenderer() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->ConfigureRenderer) mod->ConfigureRenderer();
	});
}

void Renderer::InitLayouts() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->InitVulkanLayouts) mod->InitVulkanLayouts();
	});
}

void Renderer::ConfigureShaders() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->ConfigureShaders) mod->ConfigureShaders();
	});
}

// Scene
void Renderer::ReadShaders() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->ReadShaders) mod->ReadShaders();
	});
}

// Resources
void Renderer::CreateResources() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->CreateVulkanResources) mod->CreateVulkanResources();
	});
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->CreateVulkanResources2) mod->CreateVulkanResources2(renderingDevice);
	});
}

void Renderer::DestroyResources() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->DestroyVulkanResources2) mod->DestroyVulkanResources2(renderingDevice);
	});
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->DestroyVulkanResources) mod->DestroyVulkanResources();
	});
}

void Renderer::AllocateBuffers() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->AllocateVulkanBuffers) mod->AllocateVulkanBuffers();
	});
}

void Renderer::FreeBuffers() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->FreeVulkanBuffers) mod->FreeVulkanBuffers();
	});
}

// Pipelines
void Renderer::CreatePipelines() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->CreateVulkanPipelines) mod->CreateVulkanPipelines();
	});
}

void Renderer::DestroyPipelines() {
	// Modules
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->DestroyVulkanPipelines) mod->DestroyVulkanPipelines();
	});
}

void Renderer::Update(double deltaTime) {
	this->previousDeltaTime = this->deltaTime;
	this->deltaTime = deltaTime;
	avgDeltaTime = glm::mix(avgDeltaTime, deltaTime, 1.0/10);
	renderThreadId = std::this_thread::get_id();
	std::scoped_lock lock(renderMutex1);
	
	if (!graphicsLoadedToDevice || mustReload) {
		ReloadRenderer();
		return;
	}
	
	renderingDevice->AllocatorSetCurrentFrameIndex(0);
	
	V4D_Mod::ForEachSortedModule([this](auto mod){
		if (mod->RenderUpdate) mod->RenderUpdate();
	});
	
	++frameIndex;
}

#pragma endregion

#pragma region Pack Helpers
namespace v4d::graphics {
	
	glm::f32 PackColorAsFloat(glm::vec4 color) {
		color *= 255.0f;
		glm::uvec4 pack {
			glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
		};
		return glm::uintBitsToFloat((pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a);
	}

	glm::u32 PackColorAsUint(glm::vec4 color) {
		color *= 255.0f;
		glm::uvec4 pack {
			glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
		};
		return (pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a;
	}

	glm::f32 PackUVasFloat(glm::vec2 uv) {
		uv *= 65535.0f;
		glm::uvec2 pack {
			glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
			glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
		};
		return glm::uintBitsToFloat((pack.s << 16) | pack.t);
	}

	glm::u32 PackUVasUint(glm::vec2 uv) {
		uv *= 65535.0f;
		glm::uvec2 pack {
			glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
			glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
		};
		return (pack.s << 16) | pack.t;
	}

	glm::vec4 UnpackColorFromFloat(glm::f32 color) {
		glm::u32 packed = glm::floatBitsToUint(color);
		return glm::vec4(
			(packed & 0xff000000) >> 24,
			(packed & 0x00ff0000) >> 16,
			(packed & 0x0000ff00) >> 8,
			(packed & 0x000000ff) >> 0
		) / 255.0f;
	}

	glm::vec4 UnpackColorFromUint(glm::u32 color) {
		return glm::vec4(
			(color & 0xff000000) >> 24,
			(color & 0x00ff0000) >> 16,
			(color & 0x0000ff00) >> 8,
			(color & 0x000000ff) >> 0
		) / 255.0f;
	}

	glm::vec2 UnpackUVfromFloat(glm::f32 uv) {
		glm::u32 packed = glm::floatBitsToUint(uv);
		return glm::vec2(
			(packed & 0xffff0000) >> 16,
			(packed & 0x0000ffff) >> 0
		) / 65535.0f;
	}

	glm::vec2 UnpackUVfromUint(glm::u32 uv) {
		return glm::vec2(
			(uv & 0xffff0000) >> 16,
			(uv & 0x0000ffff) >> 0
		) / 65535.0f;
	}
}
#pragma endregion
