#include "ShaderBindingTable.h"

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::raytracing;

ShaderBindingTable::ShaderBindingTable(PipelineLayoutObject* pipelineLayout, const char* shaderFile) : shaderPipelineMetaFile(shaderFile, this), pipelineLayout(pipelineLayout) {
	std::vector<ShaderInfo> files = shaderPipelineMetaFile;
	
	ShaderInfo rchit {""};
	ShaderInfo rahit {""};
	ShaderInfo rint {""};
	
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
			rchit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rachit") {
			rahit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rint") {
			rint = shaderFile;
		}
	}
	if (rchit.filepath != "" || rahit.filepath != "") {
		AddHitShader(rchit, rahit, rint);
	}
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
	
	ShaderInfo rchit {""};
	ShaderInfo rahit {""};
	ShaderInfo rint {""};
	
	for (auto& shaderFile : files) {
		v4d::io::FilePath filePath(shaderFile.filepath);
		if (filePath.GetExtension() == ".rchit") {
			rchit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rahit") {
			rahit = shaderFile;
		}
		else if (filePath.GetExtension() == ".rint") {
			rint = shaderFile;
		}
	}
	
	return AddHitShader(rchit, rahit, rint);
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
	
	groups.reserve(rayGenGroups.size() + rayMissGroups.size() + rayHitGroups.size());
	for (auto& group : rayGenGroups) groups.push_back(group);
	for (auto& group : rayMissGroups) groups.push_back(group);
	for (auto& group : rayHitGroups) groups.push_back(group);
	for (auto& group : rayCallableGroups) groups.push_back(group);
	
	assert(groups.size() > 0);
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
		rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineInfo.stageCount = (uint)stages.size();
		rayTracingPipelineInfo.pStages = stages.data();
		rayTracingPipelineInfo.groupCount = (uint)groups.size();
		rayTracingPipelineInfo.pGroups = groups.data();
		rayTracingPipelineInfo.maxPipelineRayRecursionDepth = 1;
		rayTracingPipelineInfo.layout = pipelineLayout->obj;
		
		// const VkRayTracingPipelineInterfaceCreateInfoKHR*    pLibraryInterface;
		// VkPipeline                                           basePipelineHandle;
		// int32_t                                              basePipelineIndex;
		
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	return pipeline;
}

void ShaderBindingTable::DestroyRayTracingPipeline() {
	if (device) {
		device->DestroyPipeline(pipeline, nullptr);
		DestroyShaderStages();
		bufferOffset = 0;
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
	
	auto addAlignedShaderRegion = [this](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = bufferSize;
		bufferSize += size = n * rayTracingPipelineProperties.shaderGroupHandleSize;
		// Align
		if (rayTracingPipelineProperties.shaderGroupBaseAlignment > 0) {
			if (bufferSize <= rayTracingPipelineProperties.shaderGroupBaseAlignment) {
				bufferSize = rayTracingPipelineProperties.shaderGroupBaseAlignment;
			} else if (bufferSize % rayTracingPipelineProperties.shaderGroupBaseAlignment > 0) {
				bufferSize += rayTracingPipelineProperties.shaderGroupBaseAlignment - (bufferSize % rayTracingPipelineProperties.shaderGroupBaseAlignment);
			}
		}
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	addAlignedShaderRegion(rayCallableShaderRegionOffset, rayCallableShaderRegionSize, rayCallableGroups.size());
	
	bufferSize += rayTracingPipelineProperties.shaderGroupBaseAlignment;
	
	return bufferSize;
}

void ShaderBindingTable::WriteShaderBindingTableToBuffer() {
	uint32_t sbtSize = GetSbtBufferSize(); // This is called first when creating the buffer
	VkDeviceSize bindingStride = rayTracingPipelineProperties.shaderGroupHandleSize;
	
	VkDeviceAddress baseAddress = *buffer;
	// Align
	if (rayTracingPipelineProperties.shaderGroupBaseAlignment > 0) {
		bufferOffset += rayTracingPipelineProperties.shaderGroupBaseAlignment - (baseAddress % rayTracingPipelineProperties.shaderGroupBaseAlignment);
	}
	
	// Ray Gen
	rayGenDeviceAddressRegion = {
		baseAddress + bufferOffset + rayGenShaderRegionOffset,
		bindingStride, 
		rayGenShaderRegionSize
	};
	
	// Ray Miss
	rayMissDeviceAddressRegion = {
		baseAddress + bufferOffset + rayMissShaderRegionOffset,
		bindingStride,
		rayMissShaderRegionSize
	};
	
	// Ray Hit
	rayHitDeviceAddressRegion = {
		baseAddress + bufferOffset + rayHitShaderRegionOffset,
		bindingStride,
		rayHitShaderRegionSize
	};
	
	// Ray Callable
	rayCallableDeviceAddressRegion = {
		baseAddress + bufferOffset + rayCallableShaderRegionOffset,
		bindingStride,
		rayCallableShaderRegionSize
	};
	
	uint8_t* data;
	device->MapMemoryAllocation(buffer->allocation, (void**)&data);
	const size_t shaderHandlerStorageSize = groups.size()*rayTracingPipelineProperties.shaderGroupHandleSize;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)groups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	int shaderHandlerStorageOffset = 0;
	
	// Ray Gen
	memcpy(data + bufferOffset + rayGenShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayGenGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayGenGroups.size();
	
	// Ray Miss
	memcpy(data + bufferOffset + rayMissShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayMissGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayMissGroups.size();
	
	// Ray Hit
	memcpy(data + bufferOffset + rayHitShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayHitGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayHitGroups.size();
	
	// Ray Callable
	memcpy(data + bufferOffset + rayCallableShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayCallableGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayCallableGroups.size();
	
	device->UnmapMemoryAllocation(buffer->allocation);
	delete[] shaderHandleStorage;
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
		buffer = std::make_unique<BufferObject>(MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, GetSbtBufferSize());
		buffer->Allocate(device);
		assert(buffer);
		WriteShaderBindingTableToBuffer();
	}
}

void ShaderBindingTable::Destroy() {
	if (device) {
		buffer = nullptr;
		DestroyRayTracingPipeline();
		device = nullptr;
	}
}
