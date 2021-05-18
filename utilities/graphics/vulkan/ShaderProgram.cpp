#include "ShaderProgram.h"

using namespace v4d::graphics::vulkan;

ShaderProgram::ShaderProgram(const std::vector<ShaderInfo>& infos) {
	for (auto& info : infos)
		shaderFiles.push_back(info);
}

ShaderProgram::~ShaderProgram() {}

void ShaderProgram::ReadShaders() {
	shaders.clear();
	for (auto& shader : shaderFiles) {
		auto& s = shaders.emplace_back(shader.filepath, shader.entryPoint);
		if (specializations.contains(s.type)) {
			s.specialization = specializations[s.type];
		}
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
