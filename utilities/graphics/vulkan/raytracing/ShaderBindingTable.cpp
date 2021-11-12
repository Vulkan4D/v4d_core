#include "ShaderBindingTable.h"

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::raytracing;

ShaderBindingTable::ShaderBindingTable(PipelineLayoutObject* pipelineLayout, const char* shaderFile) : shaderPipelineMetaFile(shaderFile, this), pipelineLayout(pipelineLayout) {
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

VkPipeline ShaderBindingTable::GetPipeline() const {
	return pipeline;
}

PipelineLayoutObject* ShaderBindingTable::GetPipelineLayout() const {
	return pipelineLayout;
}

void ShaderBindingTable::PushConstant(VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	assert(device);
	auto& pushConstantRange = pipelineLayout->pushConstants[pushConstantIndex];
	device->CmdPushConstants(cmdBuffer, pipelineLayout->obj, pushConstantRange.stageFlags, pushConstantRange.offset, pushConstantRange.size, pushConstant);
}

std::vector<VkPipelineShaderStageCreateInfo> ShaderBindingTable::GetStages() const {
	return stages;
}

// Rules: 
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then generalShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_SHADER_STAGE_MISS_BIT_KHR, or VK_SHADER_STAGE_CALLABLE_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then closestHitShader, anyHitShader, and intersectionShader must be VK_SHADER_UNUSED_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR then intersectionShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_INTERSECTION_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR then intersectionShader must be VK_SHADER_UNUSED_KHR
	// closestHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
	// anyHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_ANY_HIT_BIT_KHR

uint32_t ShaderBindingTable::GetOrAddShaderFileIndex(const ShaderInfo& shader) {
	uint32_t index = 0;
	for (auto&[i, s] : shaderFiles) {
		assert(i == index);
		if (s.filepath == shader.filepath) {
			break;
		}
		index++;
	}
	if (index == shaderFiles.size()) {
		shaderFiles.emplace(index, shader);
	}
	return index;
}

uint32_t ShaderBindingTable::AddMissShader(const ShaderInfo& rmiss) {
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

uint32_t ShaderBindingTable::AddHitShader(const char* filePath) {
	std::vector<ShaderInfo> files = ShaderPipelineMetaFile{filePath, this};
	
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

uint32_t ShaderBindingTable::AddHitShaders(const RayTracingShaderPrograms& shaders) {
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

uint32_t ShaderBindingTable::AddHitShader(const ShaderInfo& rchit, const ShaderInfo& rahit, const ShaderInfo& rint) {
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

uint32_t ShaderBindingTable::AddCallableShader(const ShaderInfo& rcall) {
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

void ShaderBindingTable::ReadShaders() {
	shaderObjects.clear();
	for (auto&[i, shader] : shaderFiles) {
		shaderObjects.emplace_back(shader.filepath, shader.entryPoint);
	}
}

void ShaderBindingTable::CreateShaderStages() {
	if (device && stages.size() == 0) {
		for (auto& shader : shaderObjects) {
			shader.CreateShaderModule(device);
			stages.push_back(shader.stageInfo);
		}
	}
}

void ShaderBindingTable::DestroyShaderStages() {
	if (device && stages.size() > 0) {
		stages.clear();
		for (auto& shader : shaderObjects) {
			shader.DestroyShaderModule(device);
		}
	}
}

void ShaderBindingTable::Reload() {
	if (this->device) {
		LOG("Reloading Ray Tracing Pipeline...")
		Device* device = this->device;
		Destroy();
		Create(device);
	}
}

VkPipeline ShaderBindingTable::CreateRayTracingPipeline() {
	assert(device);
	
	ReadShaders();
	CreateShaderStages();
	
	assert(stages.size() > 0);
	assert(rayGenGroups.size() == 1);
	size_t nbShaders = rayGenGroups.size() + rayMissGroups.size() + rayHitGroups.size() + rayCallableGroups.size();
	assert(stages.size() == shaderFiles.size());
	assert(stages.size() == shaderObjects.size());
	
	groups.reserve(nbShaders);
	for (auto& group : rayGenGroups) groups.push_back(group);
	for (auto& group : rayMissGroups) groups.push_back(group);
	for (auto& group : rayHitGroups) groups.push_back(group);
	for (auto& group : rayCallableGroups) groups.push_back(group);
	
	assert(groups.size() == nbShaders);
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
		rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineInfo.stageCount = (uint)stages.size();
		rayTracingPipelineInfo.pStages = stages.data();
		rayTracingPipelineInfo.groupCount = (uint)groups.size();
		rayTracingPipelineInfo.pGroups = groups.data();
		rayTracingPipelineInfo.maxPipelineRayRecursionDepth = 1;
		rayTracingPipelineInfo.layout = pipelineLayout->obj;
		// rayTracingPipelineInfo.flags = 
		
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	return pipeline;
}

void ShaderBindingTable::DestroyRayTracingPipeline() {
	if (device) {
		device->DestroyPipeline(pipeline, nullptr);
		DestroyShaderStages();
		bufferSize = 0;
		groups.clear();
	}
}

VkStridedDeviceAddressRegionKHR* ShaderBindingTable::GetRayGenDeviceAddressRegion() {
	return &rayGenDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* ShaderBindingTable::GetRayMissDeviceAddressRegion() {
	return &rayMissDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* ShaderBindingTable::GetRayHitDeviceAddressRegion() {
	return &rayHitDeviceAddressRegion;
}
VkStridedDeviceAddressRegionKHR* ShaderBindingTable::GetRayCallableDeviceAddressRegion() {
	return &rayCallableDeviceAddressRegion;
}

VkDeviceSize ShaderBindingTable::GetSbtBufferSize() {
	if (bufferSize > 0) return bufferSize;
	
	const uint32_t& handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t& alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	
	auto addAlignedShaderRegion = [this,stride](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = bufferSize;
		size = stride * n;
		bufferSize += size;
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	addAlignedShaderRegion(rayCallableShaderRegionOffset, rayCallableShaderRegionSize, rayCallableGroups.size());
	
	bufferSize += alignment; // to allow base address alignment
	
	return bufferSize;
}

void ShaderBindingTable::WriteShaderBindingTableToBuffer() {
	uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	uint64_t alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	assert(stride >= handleSize);
	assert(stride >= alignment);
	
	VkDeviceAddress baseAddress = *buffer;
	// Align
	size_t bufferOffset = 0;
	if (alignment > 0) {
		bufferOffset = ((baseAddress + (alignment - 1)) & ~(alignment - 1)) - baseAddress;
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
	
	// Ray Hit
	rayHitDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayHitShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayHitShaderRegionSize
	};
	
	// Ray Callable
	rayCallableDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayCallableShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayCallableShaderRegionSize
	};
	
	assert(groups.size() > 0);
	const size_t shaderHandlerStorageSize = groups.size()*stride;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)groups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	buffer->ZeroInitialize();
	
	// Ray Gen
	buffer->Fill(
		/*SrcPtr*/shaderHandleStorage,
		/*SizeBytes*/handleSize,
		/*OffsetBytes*/bufferOffset + rayGenShaderRegionOffset
	);
	
	uint32_t shaderHandlerStorageOffset = handleSize * rayGenGroups.size();
	
	// Ray Miss
	for (size_t i = 0; i < rayMissGroups.size(); ++i) {
		buffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayMissShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Hit
	for (size_t i = 0; i < rayHitGroups.size(); ++i) {
		buffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayHitShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Callable
	for (size_t i = 0; i < rayCallableGroups.size(); ++i) {
		buffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayCallableShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	delete[] shaderHandleStorage;
	dirty = true;
}

void ShaderBindingTable::Configure(v4d::graphics::Renderer* renderer, Device* device) {
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

void ShaderBindingTable::Create(Device* device) {
	if (this->device == nullptr) {
		this->device = device;
		CreateRayTracingPipeline();
		buffer = std::make_unique<StagingBuffer<uint8_t>>(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, GetSbtBufferSize());
		buffer->Allocate(device);
		assert(buffer);
		WriteShaderBindingTableToBuffer();
	}
}

void ShaderBindingTable::Push(VkCommandBuffer cmdBuffer) {
	assert(buffer);
	if (dirty) {
		buffer->Push(cmdBuffer);
		dirty = false;
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

void ShaderBindingTable::Destroy() {
	if (device) {
		buffer = nullptr;
		DestroyRayTracingPipeline();
		device = nullptr;
	}
}
