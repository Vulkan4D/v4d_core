#include "Renderer.h"
#include "utilities/graphics/vulkan/DescriptorSetObject.h"
#include "utilities/graphics/vulkan/raytracing/RayTracingPipeline.h"

using namespace v4d::graphics;

Device* Renderer::mainRenderingDevice = nullptr;

#pragma region Virtual INIT Methods

void Renderer::CreateDevices() {
	LOG("Finding a suitable PhysicalDevice...")
	
	// Prepare enabled extensions
	requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		#ifdef V4D_VULKAN_USE_VMA
			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
			VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
		#endif
	};
	optionalDeviceExtensions = {};
	ConfigureDeviceExtensions();
	
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
	
	// Presentation queue
	queues[0][0].surface = &surface;
	
	// Prepare Device Features
	PhysicalDevice::DeviceFeatures enabledDeviceFeatures {};
	ConfigureDeviceFeatures(&enabledDeviceFeatures, &renderingPhysicalDevice->deviceFeatures);
	renderingPhysicalDevice->deviceFeatures &= enabledDeviceFeatures;
	
	// Create Logical Device
	renderingDevice = new Device(
		renderingPhysicalDevice,
		deviceExtensions,
		vulkanLoader->requiredInstanceLayers,
		queues,
		renderingPhysicalDevice->deviceFeatures.GetDeviceFeaturesPNext()
	);
	
	mainRenderingDevice = renderingDevice;

	renderingDevice->CreateAllocator();
}

void Renderer::DestroyDevices() {
	renderingDevice->DeviceWaitIdle();
	renderingDevice->DestroyAllocator();
	mainRenderingDevice = nullptr;
	delete renderingDevice;
}

void Renderer::CreateDescriptorSets() {
	std::map<VkDescriptorType, uint> descriptorTypes {};
	
	#ifdef _ENABLE_IMGUI
		descriptorTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER]++;
	#endif
	
	// Create Descriptor Set Layout
	DescriptorSetObject::ForEach([this, &descriptorTypes](auto*set){
		set->CreateDescriptorSetLayout(renderingDevice);
		for (auto&[binding, descriptor] : set->descriptorBindings) {
			descriptorTypes[descriptor->GetDescriptorType()] += descriptor->count;
		}
	});
	
	// Create Descriptor Pool
	if (descriptorTypes.size() > 0) {
		renderingDevice->CreateDescriptorPool(
			descriptorTypes,
			descriptorPool,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
		);
	}
	
	// Allocate Descriptor Set
	DescriptorSetObject::ForEach([this](auto*set){
		set->Allocate(descriptorPool);
	});
}

void Renderer::DestroyDescriptorSets() {
	
	// Free Descriptor Set
	DescriptorSetObject::ForEach([this](auto*set){
		set->Free(descriptorPool);
	});
	
	// Destroy Descriptor Pool
	if (descriptorPool) {
		renderingDevice->DestroyDescriptorPool(descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}
	
	// Destroy Descriptor Set Layout
	DescriptorSetObject::ForEach([](auto*set){
		set->DestroyDescriptorSetLayout();
	});
}

void Renderer::UpdateDescriptorSets() {
	DescriptorSetObject::ForEach([this](auto*o){
		if (state != STATE::RUNNING || o->isDirty) {
			auto descriptorWrites = o->GetUpdateWrites();
			if (descriptorWrites.size() > 0) {
				renderingDevice->UpdateDescriptorSets((uint)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
			}
		}
	});
}

void Renderer::CreateSwapChain() {
	
	// Put old swapchain in a temporary pointer and delete it after creating new swapchain
	SwapChain* oldSwapChain = swapChain;
	swapChain = nullptr;

	do {
		if (swapChain) {
			delete swapChain;
			// #ifdef XVK_INCLUDE_GLFW
			// 	glfwWaitEvents(); // must run on main thread...
			// #else
				SLEEP(20ms)
			// #endif
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
	
	// Transition swapchain images to PRESENT layout
	RunSingleTimeCommands(queues[0][0], [this](VkCommandBuffer cmdBuffer){
		for (auto& img : swapChain->images) {
			ImageObject::TransitionImageLayout(renderingDevice, cmdBuffer, img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	});
}

void Renderer::DestroySwapChain() {
	swapChain->Destroy();
	delete swapChain;
	swapChain = nullptr;
}

#pragma endregion

#pragma region Init/Load/Reset Methods

void Renderer::WatchModifiedShadersForReload(const std::vector<ShaderPipelineMetaFile>& shaderWatchers) {
	shaderWatcherThreads.emplace_back(std::make_unique<std::thread>([watchers=shaderWatchers,this]() mutable {
		THREAD_NAME("RastrShadrWatch")
		std::vector<ShaderPipelineObject*> shadersToReload {};
		std::vector<RayTracingPipeline*> sbtsToReload {};
		while (state != STATE::NONE) {
			for (auto& watcher : watchers) {
				if (watcher.mtime == 0) {
					watcher.mtime = watcher.file.GetLastWriteTime();
				} else if (watcher.file.GetLastWriteTime() > watcher.mtime) {
					watcher.mtime = 0;
					for (auto* s : watcher.shaders) shadersToReload.push_back(s);
					if (watcher.sbt) sbtsToReload.push_back(watcher.sbt);
				}
			}
			if (shadersToReload.size() > 0 || sbtsToReload.size() > 0) {
				RunSynchronized([=,this](){
					ReloadShaderPipelines();
					for (auto* s : shadersToReload) s->Reload();
					for (auto* s : sbtsToReload) s->Reload();
					LOG("Reloaded modified shaders")
				});
				shadersToReload.clear();
				sbtsToReload.clear();
			}
			SLEEP(100ms)
		}
	}));
}

void Renderer::WatchModifiedShadersForReload(RayTracingPipeline& rtPipeline) {
	shaderWatcherThreads.emplace_back(std::make_unique<std::thread>([&rtPipeline,this]() mutable {
		THREAD_NAME("RtShadrWatch")
		double rgenModifiedTime = 0;
		while (state != STATE::NONE) {
			if (!rtPipeline.shadersDirty) {
				{
					std::lock_guard<std::mutex> lock(rtPipeline.sharedHitGroupsMutex);
					for (auto& [filePath, hitGroup] : rtPipeline.sharedHitGroups) {
						if (hitGroup.modifiedTime == 0) {
							hitGroup.modifiedTime = v4d::io::FilePath(filePath+".meta").GetLastWriteTime();
						} else if (hitGroup.modifiedTime < v4d::io::FilePath(filePath+".meta").GetLastWriteTime()) {
							hitGroup.modifiedTime = 0;
							rtPipeline.shadersDirty = true;
						}
					}
				}
				if (rgenModifiedTime == 0) {
					rgenModifiedTime = ShaderPipelineMetaFile(rtPipeline).file.GetLastWriteTime();
				} else if (rgenModifiedTime < ShaderPipelineMetaFile(rtPipeline).file.GetLastWriteTime()) {
					rgenModifiedTime = 0;
					rtPipeline.shadersDirty = true;
				}
			}
			SLEEP(100ms)
		}
	}));
}

void Renderer::RecreateSwapChain() {
	UnloadGraphicsFromDevice();
	CreateSwapChain();
	LoadGraphicsToDevice();
}

void Renderer::InitRenderer() {
	state = STATE::INITIALIZED;
	ConfigureRenderer();
	ConfigureLayouts();
	ConfigureShaders();
	ReadShaders();
}

void Renderer::LoadRenderer() {
	CreateDevices();
	CreateCommandPools();
	ConfigureScene();
	LoadBuffers();
	LoadTextures();
	CreateSwapChain();
	LoadGraphicsToDevice();
	
	LOG_SUCCESS("Vulkan Renderer is Ready !")
}

void Renderer::UnloadRenderer() {
	if (renderingDevice) {
		renderingDevice->DeviceWaitIdle();
		UnloadGraphicsFromDevice();
		DestroySwapChain();
		UnloadTextures();
		UnloadBuffers();
		DestroyCommandPools();
		DestroyDevices();
	}
}

void Renderer::ReloadShaderPipelines(bool forceReloadAllShaders) {
	renderingDevice->DeviceWaitIdle();
	if (forceReloadAllShaders) {
		LOG("Reloading all shaders...")
		DestroyShaderPipelines();
		ReadShaders();
		CreateShaderPipelines();
	}
}

void Renderer::LoadGraphicsToDevice() {
	CreateSyncObjects();
	ConfigureImages(swapChain->extent.width, swapChain->extent.height);
	CreateImages();
	CreateDescriptorSets();
	CreatePipelineLayouts();
	ConfigureRenderPasses();
	CreateRenderPasses();
	CreateShaderPipelines();
	UpdateDescriptorSets();
	CreateCommandBuffers();
	
	Start();
	
	state = STATE::LOADED;
}

void Renderer::UnloadGraphicsFromDevice() {
	state = STATE::UNLOADED;
	renderingDevice->DeviceWaitIdle();
	
	End();
	
	DestroyCommandBuffers();
	
	DestroyShaderPipelines();
	DestroyRenderPasses();
	DestroyPipelineLayouts();
	DestroyDescriptorSets();
	DestroyImages();
	DestroySyncObjects();
}

bool Renderer::BeginFrame(VkSemaphore signalSemaphore, VkFence triggerFence) {
	state = STATE::RUNNING;
	
	BeginFrame:
	
	{// Sync queue
		std::unique_lock lock1(frameSyncMutex);
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
		1000UL * 1000 * 5000, // timeout in nanoseconds
		signalSemaphore,
		triggerFence,
		&swapChainImageIndex // output the index of the swapchain image in there
	);
	switch (result) {
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
		{
			RecreateSwapChain();
			return false;
		}
		case VK_TIMEOUT:
		case VK_NOT_READY:
		{
			// This is normal behaviour in FIFO present mode
			goto BeginFrame;
		}
		default:
			throw std::runtime_error(std::string("Failed to acquire next swap chain image: ") + GetVkResultText(result));
	}
	
	return true;
}

void Renderer::RecordAndSubmitCommandBuffer(
	const VkCommandBuffer& commandBuffer,
	const VkQueue& queue,
	const std::function<void(VkCommandBuffer)>& commandsToRecord,
	const std::vector<VkSemaphore>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<VkSemaphore>& signalSemaphores,
	const VkFence& triggerFence,
	const std::vector<uint64_t> waitTimelineSemaphoreValues,
	const std::vector<uint64_t> signalTimelineSemaphoreValues
) {
	assert(waitSemaphores.size() == waitStages.size());
	assert(waitTimelineSemaphoreValues.size() == 0 || waitTimelineSemaphoreValues.size() == waitSemaphores.size());
	assert(signalTimelineSemaphoreValues.size() == 0 || signalTimelineSemaphoreValues.size() == signalSemaphores.size());
	
	VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmit {};
		timelineSemaphoreSubmit.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineSemaphoreSubmit.waitSemaphoreValueCount = waitTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pWaitSemaphoreValues = waitTimelineSemaphoreValues.data();
		timelineSemaphoreSubmit.signalSemaphoreValueCount = signalTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pSignalSemaphoreValues = signalTimelineSemaphoreValues.data();
	
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
		if (waitTimelineSemaphoreValues.size() > 0 || signalTimelineSemaphoreValues.size() > 0) {
			submitInfo.pNext = &timelineSemaphoreSubmit;
		}
	
	CheckVkResult("Reset command buffer", renderingDevice->ResetCommandBuffer(commandBuffer, 0));
	CheckVkResult("Begin recording command buffer", renderingDevice->BeginCommandBuffer(commandBuffer, &beginInfo));
		commandsToRecord(commandBuffer);
	CheckVkResult("End recording command buffer", renderingDevice->EndCommandBuffer(commandBuffer));
	ResetFence(triggerFence);
	CheckVkResult("Queue Submit", renderingDevice->QueueSubmit(queue, 1, &submitInfo, triggerFence));
}

void Renderer::SubmitCommandBuffer(
	CommandBufferObject& commandBuffer,
	const std::vector<VkSemaphore>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<VkSemaphore>& signalSemaphores,
	const VkFence& triggerFence,
	const std::vector<uint64_t> waitTimelineSemaphoreValues,
	const std::vector<uint64_t> signalTimelineSemaphoreValues
) {
	assert(waitSemaphores.size() == waitStages.size());
	assert(waitTimelineSemaphoreValues.size() == 0 || waitTimelineSemaphoreValues.size() == waitSemaphores.size());
	assert(signalTimelineSemaphoreValues.size() == 0 || signalTimelineSemaphoreValues.size() == signalSemaphores.size());
	
	VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmit {};
		timelineSemaphoreSubmit.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineSemaphoreSubmit.waitSemaphoreValueCount = waitTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pWaitSemaphoreValues = waitTimelineSemaphoreValues.data();
		timelineSemaphoreSubmit.signalSemaphoreValueCount = signalTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pSignalSemaphoreValues = signalTimelineSemaphoreValues.data();
	VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		if (waitTimelineSemaphoreValues.size() > 0 || signalTimelineSemaphoreValues.size() > 0) {
			submitInfo.pNext = &timelineSemaphoreSubmit;
		}
	
	ResetFence(triggerFence);
	CheckVkResult("Queue Submit", renderingDevice->QueueSubmit(commandBuffer.GetQueue().handle, 1, &submitInfo, triggerFence));
}

void Renderer::RecordIfDirtyAndSubmitCommandBuffer(
	CommandBufferObject& commandBuffer,
	std::function<void(VkCommandBuffer)>&& commandsToRecord,
	const std::vector<VkSemaphore>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<VkSemaphore>& signalSemaphores,
	const VkFence& triggerFence,
	const std::vector<uint64_t> waitTimelineSemaphoreValues,
	const std::vector<uint64_t> signalTimelineSemaphoreValues
){
	if (commandBuffer.dirty) {
		commandBuffer.dirty = false;
		
		assert(waitSemaphores.size() == waitStages.size());
		assert(waitTimelineSemaphoreValues.size() == 0 || waitTimelineSemaphoreValues.size() == waitSemaphores.size());
		assert(signalTimelineSemaphoreValues.size() == 0 || signalTimelineSemaphoreValues.size() == signalSemaphores.size());

		VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
		
		CheckVkResult("Reset command buffer", renderingDevice->ResetCommandBuffer(commandBuffer, 0));
		CheckVkResult("Begin recording command buffer", renderingDevice->BeginCommandBuffer(commandBuffer, &beginInfo));
			commandsToRecord(commandBuffer);
		CheckVkResult("End recording command buffer", renderingDevice->EndCommandBuffer(commandBuffer));
	}
	
	VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmit {};
		timelineSemaphoreSubmit.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineSemaphoreSubmit.waitSemaphoreValueCount = waitTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pWaitSemaphoreValues = waitTimelineSemaphoreValues.data();
		timelineSemaphoreSubmit.signalSemaphoreValueCount = signalTimelineSemaphoreValues.size();
		timelineSemaphoreSubmit.pSignalSemaphoreValues = signalTimelineSemaphoreValues.data();
	VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		if (waitTimelineSemaphoreValues.size() > 0 || signalTimelineSemaphoreValues.size() > 0) {
			submitInfo.pNext = &timelineSemaphoreSubmit;
		}
		
	ResetFence(triggerFence);
	CheckVkResult("Queue Submit", renderingDevice->QueueSubmit(commandBuffer.GetQueue().handle, 1, &submitInfo, triggerFence));
}

bool Renderer::EndFrame(const std::vector<VkSemaphore>& waitSemaphores) {
	VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain->GetHandle();
		presentInfo.pImageIndices = &swapChainImageIndex;
		
	VkResult result = renderingDevice->QueuePresentKHR(queues[0][0].handle, &presentInfo);

	// Check for errors
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		RecreateSwapChain();
		return false;
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error(std::string("Failed to present: ") + GetVkResultText(result));
	}
	
	return true;
}

#pragma endregion

#ifdef _ENABLE_IMGUI
	void Renderer::InitImGui(GLFWwindow* window) {
		imGuiGlfwWindow = window;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		imGuiIO = &ImGui::GetIO();
		imGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		ImGui::GetStyle().Alpha = 0.8f;
		ImGui_ImplGlfw_InitForVulkan(window, true);
	}
	void Renderer::LoadImGui(VkRenderPass renderPass) {
		static ImGui_ImplVulkan_InitInfo init_info {};
			init_info.Instance = handle;
			init_info.PhysicalDevice = renderingDevice->GetPhysicalDevice()->GetHandle();
			init_info.Device = renderingDevice->GetHandle();
			init_info.QueueFamily = queues[0][0].family;
			init_info.Queue = queues[0][0].handle;
			init_info.DescriptorPool = descriptorPool;
			init_info.MinImageCount = swapChain->images.size();
			init_info.ImageCount = swapChain->images.size();
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.CheckVkResultFn = [](VkResult err){if (err) LOG_ERROR("ImGui Vk Error " << (int)err)};
		ImGui_ImplVulkan_Init(&init_info, renderPass);
		// Clear any existing draw data
		auto* drawData = ImGui::GetDrawData();
		if (drawData) drawData->Clear();
		// Font Upload
		auto cmdBuffer = BeginSingleTimeCommands(queues[0][0]);
			ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		EndSingleTimeCommands(queues[0][0], cmdBuffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
	void Renderer::UnloadImGui() {
		// Clear any existing draw data
		auto* drawData = ImGui::GetDrawData();
		if (drawData) drawData->Clear();
		// Shutdown ImGui
		ImGui_ImplVulkan_Shutdown();
	}
	void Renderer::DrawImGui(VkCommandBuffer commandBuffer) {
		auto* drawData = ImGui::GetDrawData();
		if (drawData) {
			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
		}
	}
	void Renderer::BeginFrameImGui() {
		ImGui_ImplVulkan_NewFrame();
		
		ImGui_ImplGlfw_NewFrame();// this function calls glfwGetWindowAttrib() to check for focus before fetching mouse pos and always returns false if called on a secondary thread... 
		// The quick fix is simply to always fetch the mouse position right here...
		double mouse_x, mouse_y;
		glfwGetCursorPos(imGuiGlfwWindow, &mouse_x, &mouse_y);
		imGuiIO->MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		
		ImGui::NewFrame();
	}
	void Renderer::EndFrameImGui() {
		ImGui::Render();
	}
#endif

#pragma region Constructor

Renderer::Renderer(Loader* loader, const char* applicationName, uint applicationVersion)
: Instance(loader, applicationName, applicationVersion) {}

Renderer::~Renderer() {
	state = STATE::NONE;
	for (auto& shaderWatcherThread : shaderWatcherThreads) {
		if (shaderWatcherThread && shaderWatcherThread->joinable()) {
			shaderWatcherThread->join();
			shaderWatcherThread.reset();
		}
	}
	if (surface) {
		DestroySurfaceKHR(surface, nullptr);
	}
}

#pragma endregion
