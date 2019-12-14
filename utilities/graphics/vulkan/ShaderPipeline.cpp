#include <v4d.h>

using namespace v4d::graphics::vulkan;

ShaderPipeline::~ShaderPipeline() {
	
};

void ShaderPipeline::Execute(Device* device, VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	Bind(device, cmdBuffer);
	if (pushConstant) PushConstant(device, cmdBuffer, pushConstant, pushConstantIndex);
	Render(device, cmdBuffer);
}

void ShaderPipeline::PushConstant(Device* device, VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	auto& pushConstantRange = GetPipelineLayout()->pushConstants[pushConstantIndex];
	device->CmdPushConstants(cmdBuffer, GetPipelineLayout()->handle, pushConstantRange.stageFlags, pushConstantRange.offset, pushConstantRange.size, pushConstant);
}

