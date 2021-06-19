#include "ComputeShaderPipelineObject.h"

using namespace v4d::graphics::vulkan;

void ComputeShaderPipelineObject::Create(Device* device) {
	assert(this->device == nullptr);
	this->device = device;
	shaderProgram.CreateShaderStages(device);
	VkComputePipelineCreateInfo computeCreateInfo {
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,// VkStructureType sType
		nullptr,// const void* pNext
		0,// VkPipelineCreateFlags flags
		shaderProgram.GetStages()->at(0),// VkPipelineShaderStageCreateInfo stage
		GetPipelineLayout()->obj,// VkPipelineLayout layout
		VK_NULL_HANDLE,// VkPipeline basePipelineHandle
		0// int32_t basePipelineIndex
	};
	device->CreateComputePipelines(VK_NULL_HANDLE, 1, &computeCreateInfo, nullptr, obj);
}

void ComputeShaderPipelineObject::Destroy() {
	if (device) {
		device->DestroyPipeline(obj, nullptr);
		shaderProgram.DestroyShaderStages(device);
		device = nullptr;
	}
}
