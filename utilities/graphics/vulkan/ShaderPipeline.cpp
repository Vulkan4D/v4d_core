#include <v4d.h>

using namespace v4d::graphics::vulkan;

ShaderPipeline::~ShaderPipeline() {
	
};

void ShaderPipeline::Execute(Device* device, VkCommandBuffer cmdBuffer, uint32_t instanceCount, void* pushConstant, int pushConstantIndex) {
	Bind(device, cmdBuffer);
	if (pushConstant) PushConstant(device, cmdBuffer, pushConstant, pushConstantIndex);
	Render(device, cmdBuffer, instanceCount);
}

void ShaderPipeline::Execute(Device* device, VkCommandBuffer cmdBuffer) {
	Bind(device, cmdBuffer);
	Render(device, cmdBuffer, 1);
}

void ShaderPipeline::PushConstant(Device* device, VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	auto& pushConstantRange = GetPipelineLayout()->pushConstants[pushConstantIndex];
	device->CmdPushConstants(cmdBuffer, GetPipelineLayout()->handle, pushConstantRange.stageFlags, pushConstantRange.offset, pushConstantRange.size, pushConstant);
}

uint ShaderPipeline::CompactIVec4ToUint(uint r, uint g, uint b, uint a) {
	return	  (r	<< 24)	// R
			| (g	<< 16)	// G
			| (b	<< 8 )	// B
			| (a		 );	// A
}

uint ShaderPipeline::CompactVec4ToUint(float r, float g, float b, float a) {
	return	  ((uint)std::round(std::clamp(r, 0.0f, 1.0f)*255.0f)	<< 24)	// R
			| ((uint)std::round(std::clamp(g, 0.0f, 1.0f)*255.0f)	<< 16)	// G
			| ((uint)std::round(std::clamp(b, 0.0f, 1.0f)*255.0f)	<< 8 )	// B
			| ((uint)std::round(std::clamp(a, 0.0f, 1.0f)*255.0f)		 );	// A
}

float ShaderPipeline::CompactVec3ToFloat(float r, float g, float b) {
	return r + g * 256.0f + b * 256.0f * 256.0f;
}

std::string ShaderPipeline::GetShaderPath(std::string type) const {
	for (auto& s : shaderFiles) {
		if (s.filepath.substr(s.filepath.length()-type.length(), type.length()) == type) {
			return s.filepath;
		}
	}
	return "";
}
