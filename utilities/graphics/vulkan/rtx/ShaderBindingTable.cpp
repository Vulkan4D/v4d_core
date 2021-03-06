#include "ShaderBindingTable.h"

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

uint32_t ShaderBindingTable::AddCallableShader(ShaderInfo rcall) {
	rayCallableGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr, // pNext
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		GetOrAddShaderFileIndex(std::move(rcall)), // generalShader
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
	for (auto& group : rayCallableGroups) groups.push_back(group);
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
		rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineInfo.stageCount = (uint)stages.size();
		rayTracingPipelineInfo.pStages = stages.data();
		rayTracingPipelineInfo.groupCount = (uint)groups.size();
		rayTracingPipelineInfo.pGroups = groups.data();
		rayTracingPipelineInfo.maxPipelineRayRecursionDepth = 1;
		rayTracingPipelineInfo.layout = pipelineLayout->handle;
		
		// const VkRayTracingPipelineInterfaceCreateInfoKHR*    pLibraryInterface;
		// VkPipeline                                           basePipelineHandle;
		// int32_t                                              basePipelineIndex;
	
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	return pipeline;
}

void ShaderBindingTable::DestroyRayTracingPipeline(Device* device) {
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
	bufferOffset = 0;
	bufferSize = 0;
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

VkDeviceSize ShaderBindingTable::GetSbtBufferSize(const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rayTracingPipelineProperties) {
	if (bufferSize > 0) return bufferSize;
	
	auto addAlignedShaderRegion = [this, &rayTracingPipelineProperties](VkDeviceSize& offset, VkDeviceSize& size, size_t n) {
		offset = bufferSize;
		bufferSize += size = n * rayTracingPipelineProperties.shaderGroupHandleSize;
		// Align
		if (rayTracingPipelineProperties.shaderGroupBaseAlignment && (bufferSize % rayTracingPipelineProperties.shaderGroupBaseAlignment) > 0) {
			bufferSize += rayTracingPipelineProperties.shaderGroupBaseAlignment - (bufferSize % rayTracingPipelineProperties.shaderGroupBaseAlignment);
		}
	};
	
	addAlignedShaderRegion(rayGenShaderRegionOffset, rayGenShaderRegionSize, rayGenGroups.size());
	addAlignedShaderRegion(rayMissShaderRegionOffset, rayMissShaderRegionSize, rayMissGroups.size());
	addAlignedShaderRegion(rayHitShaderRegionOffset, rayHitShaderRegionSize, rayHitGroups.size());
	addAlignedShaderRegion(rayCallableShaderRegionOffset, rayCallableShaderRegionSize, rayCallableGroups.size());
	
	return bufferSize;
}

void ShaderBindingTable::WriteShaderBindingTableToBuffer(Device* device, Buffer* buffer, VkDeviceSize offset, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rayTracingPipelineProperties) {
	this->bufferOffset = offset;
	uint32_t sbtSize = GetSbtBufferSize(rayTracingPipelineProperties); // GetSbtBufferSize must be called here, it caches some values used for setting regions
	VkDeviceSize bindingStride = rayTracingPipelineProperties.shaderGroupHandleSize;
	
	// Ray Gen
	rayGenDeviceAddressRegion = {
		device->GetBufferDeviceAddress(buffer->buffer) + offset + rayGenShaderRegionOffset,
		bindingStride, 
		rayGenShaderRegionSize
	};
	
	// Ray Miss
	rayMissDeviceAddressRegion = {
		device->GetBufferDeviceAddress(buffer->buffer) + offset + rayMissShaderRegionOffset,
		bindingStride,
		rayMissShaderRegionSize
	};
	
	// Ray Hit
	rayHitDeviceAddressRegion = {
		device->GetBufferDeviceAddress(buffer->buffer) + offset + rayHitShaderRegionOffset,
		bindingStride,
		rayHitShaderRegionSize
	};
	
	// Ray Callable
	rayCallableDeviceAddressRegion = {
		device->GetBufferDeviceAddress(buffer->buffer) + offset + rayCallableShaderRegionOffset,
		bindingStride,
		rayCallableShaderRegionSize
	};
	
	uint8_t* data;
	device->MapMemoryAllocation(buffer->allocation, (void**)&data, offset, sbtSize);
	const size_t shaderHandlerStorageSize = groups.size()*rayTracingPipelineProperties.shaderGroupHandleSize;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)groups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	int shaderHandlerStorageOffset = 0;
	
	// Ray Gen
	memcpy(data + rayGenShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayGenGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayGenGroups.size();
	
	// Ray Miss
	memcpy(data + rayMissShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayMissGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayMissGroups.size();
	
	// Ray Hit
	memcpy(data + rayHitShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayHitGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayHitGroups.size();
	
	// Ray Callable
	memcpy(data + rayCallableShaderRegionOffset, shaderHandleStorage + shaderHandlerStorageOffset, rayTracingPipelineProperties.shaderGroupHandleSize * rayCallableGroups.size());
	shaderHandlerStorageOffset += rayTracingPipelineProperties.shaderGroupHandleSize * rayCallableGroups.size();
	
	device->UnmapMemoryAllocation(buffer->allocation);
	delete[] shaderHandleStorage;
}
