#include "Renderer.h"
#include "utilities/io/Logger.h"

using namespace v4d::graphics;

#pragma region Static maps

	// Ray-Tracing Shaders
	std::unordered_map<std::string, uint32_t> Renderer::sbtOffsets {};

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

		ScorePhysicalDeviceSelection(score, physicalDevice);
	});

	LOG("Selected Rendering PhysicalDevice: " << renderingPhysicalDevice->GetDescription());

	// Prepare enabled extensions
	ConfigureDeviceExtensions();
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
			LOG_WARN("Unsupported Device Extension: " << ext)
		}
	}
	
	// Prepare Device Features
	PhysicalDevice::DeviceFeatures enabledDeviceFeatures {};
	ConfigureDeviceFeatures(&enabledDeviceFeatures, &renderingPhysicalDevice->deviceFeatures);
	renderingPhysicalDevice->deviceFeatures &= enabledDeviceFeatures;
	
	// Create Logical Device
	renderingDevice = new Device(
		renderingPhysicalDevice,
		deviceExtensions,
		vulkanLoader->requiredInstanceLayers,
		queuesInfo,
		renderingPhysicalDevice->deviceFeatures.GetDeviceFeaturesPNext()
	);

	if (!renderingDevice->GetPresentQueue().handle) {
		throw std::runtime_error("Failed to get Presentation Queue for surface. At least one queue must be defined with a VkSurfaceKHR or you must manually specify a 'present' queue");
	}
	
	renderingDevice->CreateAllocator();
}

void Renderer::DestroyDevices() {
	renderingDevice->DeviceWaitIdle();
	renderingDevice->DestroyAllocator();
	delete renderingDevice;
}

void Renderer::CreateCommandPools() {
	for (auto&[k, qs] : renderingDevice->GetQueues()) if (k) {
		for (auto& q : qs) {
			if (!q.commandPool) {
				renderingDevice->CreateCommandPool(q.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &q.commandPool);
			}
		}
	}
}

void Renderer::DestroyCommandPools() {
	for (auto&[k, qs] : renderingDevice->GetQueues()) if (k) {
		for (auto& q : qs) {
			if (q.commandPool) {
				renderingDevice->DestroyCommandPool(q.commandPool);
			}
		}
	}
}

void Renderer::CreateDescriptorSets() {
	
	for (auto&[name,set] : descriptorSets) {
		set->CreateDescriptorSetLayout(renderingDevice);
	}
	
	// Descriptor sets / pool
	std::map<VkDescriptorType, uint> descriptorTypes {};
	for (auto&[name,set] : descriptorSets) {
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
		for (auto&[name,set] : descriptorSets) {
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
		for (auto&[name,set] : descriptorSets) {
			descriptorSets[name]->descriptorSet = vkDescriptorSets[i++];
		}
	}
	
	UpdateDescriptorSets();
}

void Renderer::DestroyDescriptorSets() {
	// Descriptor Sets
	if (descriptorSets.size() > 0) {
		renderingDevice->FreeDescriptorSets(descriptorPool, (uint)vkDescriptorSets.size(), vkDescriptorSets.data());
		for (auto&[name,set] : descriptorSets) set->DestroyDescriptorSetLayout(renderingDevice);
		// Descriptor pools
		renderingDevice->DestroyDescriptorPool(descriptorPool, nullptr);
	}
}

void Renderer::UpdateDescriptorSets() {
	if (descriptorSets.size() > 0) {
		std::vector<VkWriteDescriptorSet> descriptorWrites {};
		for (auto&[name,set] : descriptorSets) {
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

void Renderer::CreateSwapChain() {
	
	// Put old swapchain in a temporary pointer and delete it after creating new swapchain
	SwapChain* oldSwapChain = swapChain;
	swapChain = nullptr;

	do {
		if (swapChain) {
			delete swapChain;
			#ifdef XVK_INCLUDE_GLFW
				glfwWaitEvents();
			#else
				SLEEP(20ms)
			#endif
		}
		
		VkSurfaceCapabilitiesKHR capabilities;
		GetPhysicalDeviceSurfaceCapabilitiesKHR(renderingDevice->GetPhysicalDeviceHandle(), surface, &capabilities);
		
		// Create the new swapchain object
		swapChain = new SwapChain(
			renderingDevice,
			surface,
			capabilities.currentExtent,
			preferredFormats,
			preferredPresentModes
		);
		
	} while (swapChain->extent.width == 0 || swapChain->extent.height == 0);

	// Create the swapchain handle and corresponding imageviews
	swapChain->Create(oldSwapChain);

	// Destroy the old swapchain
	if (oldSwapChain != nullptr) {
		oldSwapChain->Destroy();
		delete oldSwapChain;
	}
}

void Renderer::DestroySwapChain() {
	swapChain->Destroy();
	delete swapChain;
	swapChain = nullptr;
}

#pragma endregion

#pragma region Helper methods

// void Renderer::AllocateBufferStaged(const Queue& queue, Buffer& buffer) {
// 	auto cmdBuffer = BeginSingleTimeCommands(queue);
// 	Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
// 	stagingBuffer.srcDataPointers = std::ref(buffer.srcDataPointers);
// 	stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
// 	buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
// 	buffer.Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
// 	Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer.buffer, buffer.size);
// 	EndSingleTimeCommands(queue, cmdBuffer);
// 	stagingBuffer.Free(renderingDevice);
// }
// void Renderer::AllocateBuffersStaged(const Queue& queue, std::vector<Buffer>& buffers) {
// 	auto cmdBuffer = BeginSingleTimeCommands(queue);
// 	std::vector<Buffer> stagingBuffers {};
// 	stagingBuffers.reserve(buffers.size());
// 	for (auto& buffer : buffers) {
// 		auto& stagingBuffer = stagingBuffers.emplace_back(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
// 		stagingBuffer.srcDataPointers = std::ref(buffer.srcDataPointers);
// 		stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
// 		buffer.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
// 		buffer.Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
// 		Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer.buffer, buffer.size);
// 	}
// 	EndSingleTimeCommands(queue, cmdBuffer);
// 	for (auto& stagingBuffer : stagingBuffers) {
// 		stagingBuffer.Free(renderingDevice);
// 	}
// }
// void Renderer::AllocateBuffersStaged(const Queue& queue, std::vector<Buffer*>& buffers) {
// 	auto cmdBuffer = BeginSingleTimeCommands(queue);
// 	std::vector<Buffer> stagingBuffers {};
// 	stagingBuffers.reserve(buffers.size());
// 	for (auto& buffer : buffers) {
// 		auto& stagingBuffer = stagingBuffers.emplace_back(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
// 		stagingBuffer.srcDataPointers = std::ref(buffer->srcDataPointers);
// 		stagingBuffer.Allocate(renderingDevice, MEMORY_USAGE_CPU_ONLY, true);
// 		buffer->usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
// 		buffer->Allocate(renderingDevice, MEMORY_USAGE_GPU_ONLY, false);
// 		Buffer::Copy(renderingDevice, cmdBuffer, stagingBuffer.buffer, buffer->buffer, buffer->size);
// 	}
// 	EndSingleTimeCommands(queue, cmdBuffer);
// 	for (auto& stagingBuffer : stagingBuffers) {
// 		stagingBuffer.Free(renderingDevice);
// 	}
// }

#pragma endregion

#pragma region Init/Load/Reset Methods

void Renderer::WatchModifiedShadersForReload(const std::vector<ShaderFileWatcher>& shaderWatchers) {
	shaderWatcherThread = std::make_unique<std::thread>([watchers=shaderWatchers,this]() mutable {
		while (state != STATE::NONE) {
			for (auto& watcher : watchers) {
				if (watcher.mtime == 0) {
					watcher.mtime = watcher.file.GetLastWriteTime();
				} else if (watcher.file.GetLastWriteTime() > watcher.mtime) {
					watcher.mtime = 0;
					RunSynchronized([watcher, renderingDevice=renderingDevice](){
						for (auto& s : watcher.shaders) s->ReloadPipeline(renderingDevice);
					});
					break;
				}
			}
			SLEEP(100ms)
		}
		
	});
}

void Renderer::RecreateSwapChain() {
	UnloadGraphicsFromDevice();
	v4d::graphics::renderer::event::Resize(this);
	LoadGraphicsToDevice();
}

void Renderer::InitRenderer() {
	state = STATE::INITIALIZED;
	ConfigureLayouts();
	ConfigureShaders();
	ReadShaders();
}

void Renderer::LoadRenderer() {
	CreateDevices();
	ConfigureRenderer();
	CreateSyncObjects();
	CreateCommandPools();
	LoadGraphicsToDevice();
	
	v4d::graphics::renderer::event::Load(this);
	LOG_SUCCESS("Vulkan Renderer is Ready !")
}

void Renderer::UnloadRenderer() {
	renderingDevice->DeviceWaitIdle();
	
	v4d::graphics::renderer::event::Unload(this);
	UnloadGraphicsFromDevice();
	DestroySwapChain();
	DestroyCommandPools();
	DestroySyncObjects();
	DestroyDevices();
}

void Renderer::ReloadPipelines() {
	LOG("Reloading shaders...")
	
	renderingDevice->DeviceWaitIdle();
	
	DestroyPipelines();
	ReadShaders();
	CreatePipelines();
}

void Renderer::ReloadRenderer() {
	LOG("Reloading renderer...")
	
	renderingDevice->DeviceWaitIdle();
	
	v4d::graphics::renderer::event::Unload(this);
	UnloadGraphicsFromDevice();
	DestroySwapChain();
	DestroyCommandPools();
	DestroySyncObjects();
	DestroyDevices();
	
	v4d::graphics::renderer::event::Reload(this);
	
	ReadShaders();
	
	CreateDevices();
	ConfigureRenderer();
	CreateSyncObjects();
	CreateCommandPools();
	LoadGraphicsToDevice();
	
	v4d::graphics::renderer::event::Load(this);
	LOG_SUCCESS("Vulkan Renderer is Ready !")
}

void Renderer::LoadGraphicsToDevice() {
	CreateSwapChain();
	AllocateBuffers();
	CreateResources();
	CreateDescriptorSets();
	CreatePipelines();
	
	v4d::graphics::renderer::event::PipelinesCreate(this);
	
	CreateCommandBuffers();
	
	state = STATE::LOADED;
}

void Renderer::UnloadGraphicsFromDevice() {
	state = STATE::UNLOADED;
	renderingDevice->DeviceWaitIdle();
	
	DestroyCommandBuffers();
	
	v4d::graphics::renderer::event::PipelinesDestroy(this);
	
	DestroyPipelines();
	DestroyDescriptorSets();
	DestroyResources();
	FreeBuffers();
}

bool Renderer::BeginFrame(VkSemaphore triggerSemaphore, VkFence triggerFence) {
	state = STATE::RUNNING;
	Begin:
	
	{// Sync queue
		std::lock_guard lock(frameSyncMutex);
		if (!syncQueue.empty()) {
			renderingDevice->DeviceWaitIdle();
			while (!syncQueue.empty()) {
				syncQueue.front()();
				syncQueue.pop();
			}
		}
	}
	
	if (state != STATE::RUNNING) return false;
	
	VkResult result = renderingDevice->AcquireNextImageKHR(
		swapChain->GetHandle(), // swapChain
		1000UL * 1000 * 1000, // timeout in nanoseconds (using max disables the timeout)
		triggerSemaphore,
		triggerFence,
		&swapChainImageIndex // output the index of the swapchain image in there
	);
	switch (result) {
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
		{
			RecreateSwapChain();
			goto Begin;
		}
		case VK_TIMEOUT:
		case VK_NOT_READY:
		{
			LOG_WARN("Trying to acquire next swap chain image: " << GetVkResultText(result))
			break;
		}
		default:
			throw std::runtime_error(std::string("Failed to acquire next swap chain image: ") + GetVkResultText(result));
	}
	
	return true;
}

void Renderer::RecordAndSubmitCommandBuffer(
	const VkCommandBuffer& commandBuffer,
	const std::function<void(VkCommandBuffer)>& commandsToRecord,
	const std::vector<VkSemaphore>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<VkSemaphore>& signalSemaphores,
	const VkFence& triggerFence
) {
	assert(waitSemaphores.size() == waitStages.size());
	
	VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();
	
	ResetFence(triggerFence);
	CheckVkResult("Reset command buffer", renderingDevice->ResetCommandBuffer(commandBuffer, 0));
	CheckVkResult("Begin recording command buffer", renderingDevice->BeginCommandBuffer(commandBuffer, &beginInfo));
	
	commandsToRecord(commandBuffer);
	
	CheckVkResult("End recording command buffer", renderingDevice->EndCommandBuffer(commandBuffer));
	CheckVkResult("Queue Submit", renderingDevice->QueueSubmit(renderingDevice->GetGraphicsQueue().handle, 1, &submitInfo, triggerFence));
}

bool Renderer::EndFrame(const std::vector<VkSemaphore>& waitSemaphores) {
	VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain->GetHandle();
		presentInfo.pImageIndices = &swapChainImageIndex;
		
	VkResult result = renderingDevice->QueuePresentKHR(renderingDevice->GetPresentQueue().handle, &presentInfo);

	// Check for errors
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		RecreateSwapChain();
		return false;
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error(std::string("Failed to present: ") + GetVkResultText(result));
	}
	
	currentFrame %= NB_FRAMES_IN_FLIGHT;
	
	return true;
}

#pragma endregion

#pragma region Constructor

Renderer::Renderer(Loader* loader, const char* applicationName, uint applicationVersion)
: Instance(loader, applicationName, applicationVersion) {}

Renderer::~Renderer() {
	state = STATE::NONE;
	if (surface) {
		DestroySurfaceKHR(surface, nullptr);
	}
	if (shaderWatcherThread && shaderWatcherThread->joinable()) {
		shaderWatcherThread->join();
	}
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
