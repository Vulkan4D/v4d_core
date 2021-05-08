#include "ComputeShaderPipelineObject.h"

using namespace v4d::graphics::vulkan;

void ComputeShaderPipelineObject::Create(Device* device) {
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

void ComputeShaderPipelineObject::Destroy(Device* device) {
	device->DestroyPipeline(obj, nullptr);
	shaderProgram.DestroyShaderStages(device);
}

void ComputeShaderPipelineObject::SetGroupCounts(uint32_t x, uint32_t y, uint32_t z) {
	groupCountX = x;
	groupCountY = y;
	groupCountZ = z;
}

void ComputeShaderPipelineObject::Bind(Device* device, VkCommandBuffer cmdBuffer) {
	device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, obj);
	device->CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, GetPipelineLayout()->obj, 0, GetPipelineLayout()->vkDescriptorSets.size(), GetPipelineLayout()->vkDescriptorSets.data(), 0, nullptr);
}

void ComputeShaderPipelineObject::Render(Device* device, VkCommandBuffer cmdBuffer, uint32_t /*unused_arg*/) {
	device->CmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
}
