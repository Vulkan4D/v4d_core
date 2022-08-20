#include "RayTracingPipeline.h"

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::raytracing;

RayTracingPipeline::RayTracingPipeline(PipelineLayoutObject* pipelineLayout, const char* shaderFile) : shaderPipelineMetaFile(shaderFile, this), pipelineLayout(pipelineLayout) {
	std::vector<ShaderInfo> files = shaderPipelineMetaFile;
	
	RayTracingShaderPrograms shaders {};
	
	for (auto& shaderFile : files) {
		v4d::io::FilePath filePath(shaderFile.filepath);
		if (filePath.GetExtension() == ".rgen") {
			rayGenGroups.push_back({
				VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
				nullptr, // pNext
				VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
				GetOrAddShaderFileIndex(shaderFile), // generalShader
				VK_SHADER_UNUSED_KHR, // closestHitShader;
				VK_SHADER_UNUSED_KHR, // anyHitShader;
				VK_SHADER_UNUSED_KHR, // intersectionShader;
				nullptr // pShaderGroupCaptureReplayHandle
			});
		}
		else if (filePath.GetExtension() == ".rmiss") {
			AddMissShader(shaderFile);
		}
		else if (filePath.GetExtension() == ".rcall") {
			AddCallableShader(shaderFile);
		}
		else if (filePath.GetExtension() == ".rchit") {
			shaders[shaderFile.subpass].rchit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rahit") {
			shaders[shaderFile.subpass].rahit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rint") {
			shaders[shaderFile.subpass].rint = shaderFile;
		}
	}
	
	if (shaders.size() > 0) AddHitShaders(shaders);
}

VkPipeline RayTracingPipeline::GetPipeline() const {
	return pipeline;
}

PipelineLayoutObject* RayTracingPipeline::GetPipelineLayout() const {
	return pipelineLayout;
}

void RayTracingPipeline::PushConstant(VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	assert(device);
	auto& pushConstantRange = pipelineLayout->pushConstants[pushConstantIndex];
	device->CmdPushConstants(cmdBuffer, pipelineLayout->obj, pushConstantRange.stageFlags, pushConstantRange.offset, pushConstantRange.size, pushConstant);
}

std::vector<VkPipelineShaderStageCreateInfo> RayTracingPipeline::GetStages() const {
	return stages;
}

// Rules: 
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then generalShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_SHADER_STAGE_MISS_BIT_KHR, or VK_SHADER_STAGE_CALLABLE_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then closestHitShader, anyHitShader, and intersectionShader must be VK_SHADER_UNUSED_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR then intersectionShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_INTERSECTION_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR then intersectionShader must be VK_SHADER_UNUSED_KHR
	// closestHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
	// anyHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_ANY_HIT_BIT_KHR

uint32_t RayTracingPipeline::GetOrAddShaderFileIndex(const ShaderInfo& shader) {
	uint32_t index = 0;
	for (auto&[i, s] : shaderFiles) {
		assert(i == index);
		if (s.filepath == shader.filepath && s.specialization == shader.specialization) {
			break;
		}
		index++;
	}
	if (index == shaderFiles.size()) {
		shaderFiles.emplace(index, shader);
	}
	return index;
}

uint32_t RayTracingPipeline::AddMissShader(const ShaderInfo& rmiss) {
	rayMissGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		GetOrAddShaderFileIndex(rmiss), // generalShader
		VK_SHADER_UNUSED_KHR, // closestHitShader;
		VK_SHADER_UNUSED_KHR, // anyHitShader;
		VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
	return nextMissShaderOffset++;
}

uint32_t RayTracingPipeline::AddHitShader(const char* filePath) {
	std::vector<ShaderInfo> files = ShaderPipelineMetaFile{filePath, this};
	
	if (files.size() == 0) {
		LOG_WARN("Shader program not found at '" << filePath << "'. Using default hit group 0.")
		return 0;
	}
	
	RayTracingShaderPrograms shaders {};
	
	for (auto& shaderFile : files) {
		v4d::io::FilePath filePath(shaderFile.filepath);
		if (filePath.GetExtension() == ".rchit") {
			shaders[shaderFile.subpass].rchit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rahit") {
			shaders[shaderFile.subpass].rahit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rint") {
			shaders[shaderFile.subpass].rint = shaderFile;
		}
	}
	
	return AddHitShaders(shaders);
}

uint32_t RayTracingPipeline::AddHitShaders(const RayTracingShaderPrograms& shaders) {
	int32_t index = -1;
	for (auto& [offset, shader] : shaders) {
		uint32_t i = AddHitShader(shader.rchit, shader.rahit, shader.rint);
		if (offset == 0) {
			assert(index == -1);
			index = i;
		} else {
			assert(index >= 0);
			assert(uint32_t(index) + offset == i);
		}
	}
	assert(index >= 0);
	return uint32_t(index);
}

uint32_t RayTracingPipeline::AddHitShader(const ShaderInfo& rchit, const ShaderInfo& rahit, const ShaderInfo& rint) {
	rayHitGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		rint.filepath != "" ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
		VK_SHADER_UNUSED_KHR, // generalShader
		rchit.filepath != "" ? GetOrAddShaderFileIndex(rchit) : VK_SHADER_UNUSED_KHR, // closestHitShader;
		rahit.filepath != "" ? GetOrAddShaderFileIndex(rahit) : VK_SHADER_UNUSED_KHR, // anyHitShader;
		rint.filepath != "" ? GetOrAddShaderFileIndex(rint) : VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
	return nextHitShaderOffset++;
}

uint32_t RayTracingPipeline::AddCallableShader(const ShaderInfo& rcall) {
	rayCallableGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		GetOrAddShaderFileIndex(rcall), // generalShader
		VK_SHADER_UNUSED_KHR, // closestHitShader;
		VK_SHADER_UNUSED_KHR, // anyHitShader;
		VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
	return nextCallableShaderOffset++;
}

uint32_t RayTracingPipeline::GetOrAddHitGroup(const char* filePath) {
	std::lock_guard<std::mutex> lock(sharedHitGroupsMutex);
	if (!sharedHitGroups.contains(filePath)) {
		sharedHitGroups.emplace(filePath, AddHitShader(filePath));
		shadersDirty = true;
	}
	return sharedHitGroups.at(filePath).index;
}

void RayTracingPipeline::ReadShaders() {
	shaderObjects.clear();
	for (auto&[i, shader] : shaderFiles) {
		shaderObjects.emplace_back(shader.filepath, shader.entryPoint).specialization = shader.specialization;
	}
}

void RayTracingPipeline::CreateShaderStages() {
	if (device && stages.size() == 0) {
		for (auto& shader : shaderObjects) {
			shader.CreateShaderModule(device);
			stages.push_back(shader.stageInfo);
		}
	}
}

void RayTracingPipeline::DestroyShaderStages() {
	if (device && stages.size() > 0) {
		stages.clear();
		for (auto& shader : shaderObjects) {
			shader.DestroyShaderModule(device);
		}
	}
}

void RayTracingPipeline::Reload() {
	if (this->device) {
		LOG("Reloading Ray Tracing Pipeline...")
		Device* device = this->device;
		std::lock_guard<std::mutex> lock(sharedHitGroupsMutex);
		Destroy();
		Create(device);
	}
}

VkDeviceSize RayTracingPipeline::GetSbtBufferSize() {
	if (pipelineBufferSize > 0) return pipelineBufferSize;
	
	const uint32_t& handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t& alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	
	auto addAlignedShaderRegion = [this,stride](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = pipelineBufferSize;
		size = stride * n;
		pipelineBufferSize += size;
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayCallableShaderRegionOffset, rayCallableShaderRegionSize, rayCallableGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	
	pipelineBufferSize += alignment; // to allow base address alignment
	
	return pipelineBufferSize;
}

VkPipeline RayTracingPipeline::CreateRayTracingPipeline() {
	assert(device);
	
	ReadShaders();
	CreateShaderStages();
	
	assert(stages.size() > 0);
	assert(rayGenGroups.size() == 1);
	size_t nbShaders = rayGenGroups.size() + rayMissGroups.size() + rayCallableGroups.size() + rayHitGroups.size();
	assert(stages.size() == shaderFiles.size());
	assert(stages.size() == shaderObjects.size());
	
	allGroups.reserve(nbShaders);
	for (auto& group : rayGenGroups) {
		allGroups.push_back(group);
	}
	for (auto& group : rayMissGroups) {
		allGroups.push_back(group);
	}
	for (auto& group : rayCallableGroups) {
		allGroups.push_back(group);
	}
	for (auto& group : rayHitGroups) {
		allGroups.push_back(group);
	}
	
	assert(allGroups.size() == nbShaders);
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
		rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineInfo.stageCount = (uint)stages.size();
		rayTracingPipelineInfo.pStages = stages.data();
		rayTracingPipelineInfo.groupCount = (uint)allGroups.size();
		rayTracingPipelineInfo.pGroups = allGroups.data();
		rayTracingPipelineInfo.maxPipelineRayRecursionDepth = 8;
		rayTracingPipelineInfo.layout = pipelineLayout->obj;
		// rayTracingPipelineInfo.flags = 
		
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	pipelineBuffer = std::make_unique<StagingBuffer<uint8_t>>(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, GetSbtPipelineBufferSize());
	pipelineBuffer->Allocate(device);
	assert(pipelineBuffer);
	
	return pipeline;
}

void RayTracingPipeline::DestroyRayTracingPipeline() {
	if (device) {
		device->DestroyPipeline(pipeline, nullptr);
		DestroyShaderStages();
		pipelineBufferSize = 0;
		allGroups.clear();
	}
}

VkStridedDeviceAddressRegionKHR* RayTracingPipeline::GetRayGenDeviceAddressRegion() {
	return &rayGenDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* RayTracingPipeline::GetRayMissDeviceAddressRegion() {
	return &rayMissDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* RayTracingPipeline::GetRayHitDeviceAddressRegion() {
	return &rayHitDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* RayTracingPipeline::GetRayCallableDeviceAddressRegion() {
	return &rayCallableDeviceAddressRegion;
}

VkDeviceSize RayTracingPipeline::GetSbtPipelineBufferSize() {
	if (pipelineBufferSize > 0) return pipelineBufferSize;
	
	const uint32_t& handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t& alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	assert(stride >= handleSize);
	assert(stride >= alignment);
	
	auto addAlignedShaderRegion = [this,stride](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = pipelineBufferSize;
		size = stride * n;
		pipelineBufferSize += size;
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayCallableShaderRegionOffset, rayCallableShaderRegionSize, rayCallableGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	
	pipelineBufferSize += alignment; // to allow base address alignment
	
	return pipelineBufferSize;
}

void RayTracingPipeline::WritePipelineBuffer() {
	if (shadersDirty) {
		Reload();
	}
	
	uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	uint64_t alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	assert(stride >= handleSize);
	assert(stride >= alignment);
	
	VkDeviceAddress baseAddress = *pipelineBuffer;
	// Align
	size_t bufferOffset = 0;
	if (alignment > 0) {
		bufferOffset = ((baseAddress + (alignment - 1)) & ~(alignment - 1)) - baseAddress;
		assert(bufferOffset >= 0 && bufferOffset < alignment);
	}
	
	// Ray Gen
	rayGenDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayGenShaderRegionOffset,
		/*stride*/rayGenShaderRegionSize,
		/*size*/rayGenShaderRegionSize
	};
	
	// Ray Miss
	rayMissDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayMissShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayMissShaderRegionSize
	};
	
	// Ray Callable
	rayCallableDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayCallableShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayCallableShaderRegionSize
	};
	
	// Ray Hit
	rayHitDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayHitShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayHitShaderRegionSize
	};
	
	assert(allGroups.size() > 0);
	const size_t shaderHandlerStorageSize = allGroups.size()*handleSize;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)allGroups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	pipelineBuffer->ZeroInitialize();
	
	// Ray Gen
	pipelineBuffer->Fill(
		/*SrcPtr*/shaderHandleStorage,
		/*SizeBytes*/handleSize,
		/*OffsetBytes*/bufferOffset + rayGenShaderRegionOffset
	);
	
	uint32_t shaderHandlerStorageOffset = handleSize * rayGenGroups.size();
	
	// Ray Miss
	for (size_t i = 0; i < rayMissGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayMissShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Callable
	for (size_t i = 0; i < rayCallableGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayCallableShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Hit
	for (size_t i = 0; i < rayHitGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayHitShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	assert(shaderHandlerStorageOffset == shaderHandlerStorageSize);
	assert(bufferOffset + rayHitShaderRegionOffset + stride * rayHitGroups.size() + handleSize <= pipelineBuffer->Count());
	
	delete[] shaderHandleStorage;
	pipelineDirty = true;
}

void RayTracingPipeline::Configure(v4d::graphics::Renderer* renderer, Device* device) {
	// Query the ray tracing properties of the current implementation
	accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	accelerationStructureProperties.pNext = &rayTracingPipelineProperties;
	VkPhysicalDeviceProperties2 deviceProps2 {};{
		deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProps2.pNext = &accelerationStructureProperties;
	}
	renderer->GetPhysicalDeviceProperties2(device->GetPhysicalDevice()->GetHandle(), &deviceProps2);
}

void RayTracingPipeline::Create(Device* device) {
	shadersDirty = false;
	if (this->device == nullptr) {
		this->device = device;
		CreateRayTracingPipeline();
		WritePipelineBuffer();
	}
}

void RayTracingPipeline::Push(VkCommandBuffer cmdBuffer) {
	assert(pipelineBuffer);
	if (pipelineDirty) {
		pipelineBuffer->Push(cmdBuffer);
		pipelineDirty = false;
		{// Wait for SBT Push to finish build before tracing rays
			VkMemoryBarrier memoryBarrier {
				VK_STRUCTURE_TYPE_MEMORY_BARRIER,
				nullptr,// pNext
				VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,// VkAccessFlags srcAccessMask
				VK_ACCESS_SHADER_READ_BIT,// VkAccessFlags dstAccessMask
			};
			device->CmdPipelineBarrier(
				cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0, 1, &memoryBarrier, 0, 0, 0, 0
			);
		}
	}
}

void RayTracingPipeline::Destroy() {
	if (device) {
		pipelineBuffer = nullptr;
		DestroyRayTracingPipeline();
		device = nullptr;
	}
}
