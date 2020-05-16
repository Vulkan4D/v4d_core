#include <v4d.h>

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::rtx;

ShaderBindingTable::ShaderBindingTable(PipelineLayout& pipelineLayout, ShaderInfo rgen) : pipelineLayout(&pipelineLayout) {
	rayGenGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		GetOrAddShaderFileIndex(std::move(rgen)), // generalShader
		VK_SHADER_UNUSED_KHR, // closestHitShader;
		VK_SHADER_UNUSED_KHR, // anyHitShader;
		VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
}

VkPipeline ShaderBindingTable::GetPipeline() const {
	return pipeline;
}

PipelineLayout* ShaderBindingTable::GetPipelineLayout() const {
	return pipelineLayout;
}

// std::vector<VkRayTracingShaderGroupCreateInfoKHR> ShaderBindingTable::GetGroups() const {
// 	return groups;
// }

std::vector<VkPipelineShaderStageCreateInfo> ShaderBindingTable::GetStages() const {
	return stages;
}

// uint32_t ShaderBindingTable::GetMissGroupOffset() const {
// 	return rayGenGroups.size();
// }

// uint32_t ShaderBindingTable::GetHitGroupOffset() const {
// 	return rayGenGroups.size() + rayMissGroups.size();
// }

// Rules: 
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then generalShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_SHADER_STAGE_MISS_BIT_KHR, or VK_SHADER_STAGE_CALLABLE_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then closestHitShader, anyHitShader, and intersectionShader must be VK_SHADER_UNUSED_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR then intersectionShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_INTERSECTION_BIT_KHR
	// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR then intersectionShader must be VK_SHADER_UNUSED_KHR
	// closestHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
	// anyHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_ANY_HIT_BIT_KHR

uint32_t ShaderBindingTable::GetOrAddShaderFileIndex(ShaderInfo&& shader) {
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

uint32_t ShaderBindingTable::AddMissShader(ShaderInfo rmiss) {
	rayMissGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		GetOrAddShaderFileIndex(std::move(rmiss)), // generalShader
		VK_SHADER_UNUSED_KHR, // closestHitShader;
		VK_SHADER_UNUSED_KHR, // anyHitShader;
		VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
	return nextMissShaderOffset++;
}

uint32_t ShaderBindingTable::AddHitShader(ShaderInfo rchit, ShaderInfo rahit, ShaderInfo rint) {
	rayHitGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		rint.filepath != "" ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
		VK_SHADER_UNUSED_KHR, // generalShader
		rchit.filepath != "" ? GetOrAddShaderFileIndex(std::move(rchit)) : VK_SHADER_UNUSED_KHR, // closestHitShader;
		rahit.filepath != "" ? GetOrAddShaderFileIndex(std::move(rahit)) : VK_SHADER_UNUSED_KHR, // anyHitShader;
		rint.filepath != "" ? GetOrAddShaderFileIndex(std::move(rint)) : VK_SHADER_UNUSED_KHR, // intersectionShader;
		nullptr // pShaderGroupCaptureReplayHandle
	});
	return nextHitShaderOffset++;
}

void ShaderBindingTable::ReadShaders() {
	shaderObjects.clear();
	for (auto&[i, shader] : shaderFiles) {
		shaderObjects.emplace_back(shader.filepath, shader.entryPoint, shader.specializationInfo);
	}
}

void ShaderBindingTable::CreateShaderStages(Device* device) {
	if (stages.size() == 0) {
		for (auto& shader : shaderObjects) {
			shader.CreateShaderModule(device);
			stages.push_back(shader.stageInfo);
		}
	}
}

void ShaderBindingTable::DestroyShaderStages(Device* device) {
	if (stages.size() > 0) {
		stages.clear();
		for (auto& shader : shaderObjects) {
			shader.DestroyShaderModule(device);
		}
	}
}

VkPipeline ShaderBindingTable::CreateRayTracingPipeline(Device* device) {
	CreateShaderStages(device);
	
	groups.reserve(rayGenGroups.size() + rayMissGroups.size() + rayHitGroups.size());
	for (auto& group : rayGenGroups) groups.push_back(group);
	for (auto& group : rayMissGroups) groups.push_back(group);
	for (auto& group : rayHitGroups) groups.push_back(group);
	//TODO add callables
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
		rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineInfo.stageCount = (uint)stages.size();
		rayTracingPipelineInfo.pStages = stages.data();
		rayTracingPipelineInfo.groupCount = (uint)groups.size();
		rayTracingPipelineInfo.pGroups = groups.data();
		rayTracingPipelineInfo.maxRecursionDepth = 2;
		rayTracingPipelineInfo.layout = pipelineLayout->handle;
		rayTracingPipelineInfo.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
		
		// const VkRayTracingPipelineInterfaceCreateInfoKHR*    pLibraryInterface;
		// VkPipeline                                           basePipelineHandle;
		// int32_t                                              basePipelineIndex;
	
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	return pipeline;
}

void ShaderBindingTable::DestroyRayTracingPipeline(Device* device) {
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
	bufferOffset = 0;
	bufferSize = 0;
}

VkStridedBufferRegionKHR* ShaderBindingTable::GetRayGenBufferRegion() {
	return &rayGenBufferRegion;
}
VkStridedBufferRegionKHR* ShaderBindingTable::GetRayMissBufferRegion() {
	return &rayMissBufferRegion;
}
VkStridedBufferRegionKHR* ShaderBindingTable::GetRayHitBufferRegion() {
	return &rayHitBufferRegion;
}
VkStridedBufferRegionKHR* ShaderBindingTable::GetRayCallableBufferRegion() {
	return &rayCallableBufferRegion;
}

VkDeviceSize ShaderBindingTable::GetSbtBufferSize(const VkPhysicalDeviceRayTracingPropertiesKHR& rayTracingProperties) {
	if (bufferSize > 0) return bufferSize;
	
	auto addAlignedShaderRegion = [this, &rayTracingProperties](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = bufferSize;
		bufferSize += size = n * rayTracingProperties.shaderGroupHandleSize;
		// Align
		if (rayTracingProperties.shaderGroupBaseAlignment && (bufferSize % rayTracingProperties.shaderGroupBaseAlignment) > 0) {
			bufferSize += rayTracingProperties.shaderGroupBaseAlignment - (bufferSize % rayTracingProperties.shaderGroupBaseAlignment);
		}
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	//TODO add callables
	
	return bufferSize;
}

void ShaderBindingTable::WriteShaderBindingTableToBuffer(Device* device, Buffer* buffer, VkDeviceSize offset, const VkPhysicalDeviceRayTracingPropertiesKHR& rayTracingProperties) {
	this->bufferOffset = offset;
	uint32_t sbtSize = GetSbtBufferSize(rayTracingProperties); // GetSbtBufferSize must be called here, it caches some values used for setting regions
	VkDeviceSize bindingStride = rayTracingProperties.shaderGroupHandleSize;
	
	// Ray Gen
	rayGenBufferRegion = {
		buffer->buffer, 
		offset + rayGenShaderRegionOffset,
		bindingStride, 
		rayGenShaderRegionSize
	};
	
	// Ray Miss
	rayMissBufferRegion = {
		buffer->buffer,
		offset + rayMissShaderRegionOffset,
		bindingStride,
		rayMissShaderRegionSize
	};
	
	// Ray Hit
	rayHitBufferRegion = {
		buffer->buffer,
		offset + rayHitShaderRegionOffset,
		bindingStride,
		rayHitShaderRegionSize
	};
	
	// Ray Callable
	rayCallableBufferRegion = { //TODO implement callables
		VK_NULL_HANDLE,
		0,
		bindingStride,
		0
	};
	
	uint8_t* data;
	device->MapMemory(buffer->memory, offset, sbtSize, 0, (void**)&data);
	const size_t shaderHandlerStorageSize = groups.size()*rayTracingProperties.shaderGroupHandleSize;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)groups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	int shaderHandlerStorageOffset = 0;
	
	// Ray Gen
	memcpy(data + rayGenShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingProperties.shaderGroupHandleSize * rayGenGroups.size());
	shaderHandlerStorageOffset += rayTracingProperties.shaderGroupHandleSize * rayGenGroups.size();
	
	// Ray Miss
	memcpy(data + rayMissShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingProperties.shaderGroupHandleSize * rayMissGroups.size());
	shaderHandlerStorageOffset += rayTracingProperties.shaderGroupHandleSize * rayMissGroups.size();
	
	// Ray Hit
	memcpy(data + rayHitShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingProperties.shaderGroupHandleSize * rayHitGroups.size());
	shaderHandlerStorageOffset += rayTracingProperties.shaderGroupHandleSize * rayHitGroups.size();
	
	//TODO add callables
	
	device->UnmapMemory(buffer->memory);
	delete[] shaderHandleStorage;
}
