#include <v4d.h>

using namespace v4d::graphics::vulkan;

ShaderProgram::ShaderProgram(PipelineLayout* pipelineLayout, const std::vector<ShaderInfo>& infos) : pipelineLayout(pipelineLayout) {
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

void ShaderProgram::LoadShaders() {
	shaders.clear();
	for (auto& shader : shaderFiles) {
		shaders.emplace_back(shader.filepath, shader.entryPoint, shader.specializationInfo);
	}
}

void ShaderProgram::UnloadShaders() {
	shaders.clear();
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

void ShaderProgram::SetPipelineLayout(PipelineLayout* layout) {
	this->pipelineLayout = layout;
}

PipelineLayout* ShaderProgram::GetPipelineLayout() const {
	return pipelineLayout;
}

std::vector<VkPipelineShaderStageCreateInfo>* ShaderProgram::GetStages() {
	return &stages;
}

std::vector<VkVertexInputBindingDescription>* ShaderProgram::GetBindings() {
	return &bindings;
}

std::vector<VkVertexInputAttributeDescription>* ShaderProgram::GetAttributes() {
	return &attributes;
}
