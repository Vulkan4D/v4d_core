#include "ComputeShaderPipeline.h"

using namespace v4d::graphics::vulkan;

ComputeShaderPipeline::~ComputeShaderPipeline() {
	
}

void ComputeShaderPipeline::CreatePipeline(Device* device) {
	CreateShaderStages(device);
	VkComputePipelineCreateInfo computeCreateInfo {
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,// VkStructureType sType
		nullptr,// const void* pNext
		0,// VkPipelineCreateFlags flags
		GetStages()->at(0),// VkPipelineShaderStageCreateInfo stage
		GetPipelineLayout()->handle,// VkPipelineLayout layout
		VK_NULL_HANDLE,// VkPipeline basePipelineHandle
		0// int32_t basePipelineIndex
	};
	device->CreateComputePipelines(VK_NULL_HANDLE, 1, &computeCreateInfo, nullptr, &pipeline);
}

void ComputeShaderPipeline::DestroyPipeline(Device* device) {
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
}

void ComputeShaderPipeline::SetGroupCounts(uint32_t x, uint32_t y, uint32_t z) {
	groupCountX = x;
	groupCountY = y;
	groupCountZ = z;
}

void ComputeShaderPipeline::Bind(Device* device, VkCommandBuffer cmdBuffer) {
	device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	device->CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, GetPipelineLayout()->handle, 0, GetPipelineLayout()->vkDescriptorSets.size(), GetPipelineLayout()->vkDescriptorSets.data(), 0, nullptr);
}

void ComputeShaderPipeline::Render(Device* device, VkCommandBuffer cmdBuffer, uint32_t /*unused_arg*/) {
	device->CmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
}
