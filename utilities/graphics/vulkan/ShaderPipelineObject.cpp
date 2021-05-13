#include "ShaderPipelineObject.h"

using namespace v4d::graphics::vulkan;

COMMON_OBJECT_CPP (ShaderPipelineObject, VkPipeline)

void ShaderPipelineObject::Execute(uint32_t frameIndex, VkCommandBuffer cmdBuffer, uint32_t instanceCount, void* pushConstant, int pushConstantIndex) {
	assert(device);
	Bind(frameIndex, cmdBuffer);
	if (pushConstant) PushConstant(cmdBuffer, pushConstant, pushConstantIndex);
	Render(frameIndex, cmdBuffer, instanceCount);
}

void ShaderPipelineObject::Execute(uint32_t frameIndex, VkCommandBuffer cmdBuffer) {
	assert(device);
	Bind(frameIndex, cmdBuffer);
	Render(frameIndex, cmdBuffer, 1);
}

void ShaderPipelineObject::PushConstant(VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex) {
	assert(device);
	auto& pushConstantRange = GetPipelineLayout()->pushConstants[pushConstantIndex];
	device->CmdPushConstants(cmdBuffer, GetPipelineLayout()->obj, pushConstantRange.stageFlags, pushConstantRange.offset, pushConstantRange.size, pushConstant);
}

uint ShaderPipelineObject::CompactIVec4ToUint(uint r, uint g, uint b, uint a) {
	return	  (r	<< 24)	// R
			| (g	<< 16)	// G
			| (b	<< 8 )	// B
			| (a		 );	// A
}

uint ShaderPipelineObject::CompactVec4ToUint(float r, float g, float b, float a) {
	return	  ((uint)std::round(std::clamp(r, 0.0f, 1.0f)*255.0f)	<< 24)	// R
			| ((uint)std::round(std::clamp(g, 0.0f, 1.0f)*255.0f)	<< 16)	// G
			| ((uint)std::round(std::clamp(b, 0.0f, 1.0f)*255.0f)	<< 8 )	// B
			| ((uint)std::round(std::clamp(a, 0.0f, 1.0f)*255.0f)		 );	// A
}

float ShaderPipelineObject::CompactVec3rgb10ToFloat(float x, float y, float z) {
	union {
		uint32_t packedUint = 0;
		float packedFloat;
	};
	packedUint |= ((uint32_t)glm::round(x*1023.0f)) << 20;
	packedUint |= ((uint32_t)glm::round(y*1023.0f)) << 10;
	packedUint |= ((uint32_t)glm::round(z*1023.0f));
	return packedFloat;
}

std::string ShaderPipelineObject::GetShaderPath(std::string type) const {
	for (auto& s : shaderProgram.GetShaderFiles()) {
		if (s.filepath.substr(s.filepath.length()-type.length(), type.length()) == type) {
			return s.filepath;
		}
	}
	return "";
}
