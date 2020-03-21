#include <v4d.h>

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::rtx;

ShaderBindingTable::ShaderBindingTable(PipelineLayout& pipelineLayout, ShaderInfo rgen) : pipelineLayout(&pipelineLayout) {
	groups.push_back({
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

std::vector<VkRayTracingShaderGroupCreateInfoKHR> ShaderBindingTable::GetGroups() const {
	return groups;
}

std::vector<VkPipelineShaderStageCreateInfo> ShaderBindingTable::GetStages() const {
	return stages;
}

uint32_t ShaderBindingTable::GetHitGroupOffset() const {
	return hitGroupOffset;
}

uint32_t ShaderBindingTable::GetMissGroupOffset() const {
	return missGroupOffset;
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
	if (missGroupOffset == 0) missGroupOffset = groups.size();
	groups.push_back({
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
	if (hitGroupOffset == 0) hitGroupOffset = groups.size();
	groups.push_back({
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
	
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo {};
	rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineInfo.stageCount = (uint)stages.size();
	rayTracingPipelineInfo.pStages = stages.data();
	rayTracingPipelineInfo.groupCount = (uint)groups.size();
	rayTracingPipelineInfo.pGroups = groups.data();
	rayTracingPipelineInfo.maxRecursionDepth = 2;
	rayTracingPipelineInfo.layout = pipelineLayout->handle;
	
	if (device->CreateRayTracingPipelinesKHR(VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS) //TODO support multiple ray tracing pipelines
		throw std::runtime_error("Failed to create ray tracing pipelines");
	
	return pipeline;
}

void ShaderBindingTable::DestroyRayTracingPipeline(Device* device) {
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
}

void ShaderBindingTable::WriteShaderBindingTableToBuffer(Device* device, Buffer* buffer, uint32_t shaderGroupHandleSize) {
	uint32_t sbtSize = shaderGroupHandleSize * groups.size();
	uint8_t* data;
	device->MapMemory(buffer->memory, 0, sbtSize, 0, (void**)&data);
	auto shaderHandleStorage = new uint8_t[sbtSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)groups.size(), sbtSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	for (uint32_t groupIndex = 0; groupIndex < groups.size(); ++groupIndex) {
		memcpy(data, shaderHandleStorage + groupIndex * shaderGroupHandleSize, shaderGroupHandleSize);
		data += shaderGroupHandleSize;
	}
	
	device->UnmapMemory(buffer->memory);
	delete[] shaderHandleStorage;
}
