#include "ShaderProgram.h"

using namespace v4d::graphics::vulkan;

ShaderProgram::ShaderProgram(const std::vector<ShaderInfo>& infos) {
	for (auto& info : infos)
		shaderFiles.push_back(info);
}

ShaderProgram::~ShaderProgram() {}

void ShaderProgram::AddVertexInputBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs) {
	bindings.emplace_back(VkVertexInputBindingDescription{binding, stride, inputRate});
	for (auto attr : attrs) {
		attributes.emplace_back(VkVertexInputAttributeDescription{attr.location, binding, attr.format, attr.offset});
	}
}

void ShaderProgram::AddVertexInputBinding(uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs) {
	AddVertexInputBinding(bindings.size(), stride, inputRate, attrs);
}

void ShaderProgram::ReadShaders() {
	shaders.clear();
	for (auto& shader : shaderFiles) {
		shaders.emplace_back(shader.filepath, shader.entryPoint, shader.specializationInfo);
	}
}

void ShaderProgram::CreateShaderStages(Device* device) {
	if (stages.size() == 0) {
		for (auto& shader : shaders) {
			shader.CreateShaderModule(device);
			stages.push_back(shader.stageInfo);
		}
	}
}

void ShaderProgram::DestroyShaderStages(Device* device) {
	if (stages.size() > 0) {
		stages.clear();
		for (auto& shader : shaders) {
			shader.DestroyShaderModule(device);
		}
	}
}
