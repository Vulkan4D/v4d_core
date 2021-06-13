#include <regex>
#include <fstream>
#include "Shader.h"

using namespace v4d::graphics::vulkan;

Shader::Shader(std::string filepath, std::string entryPoint)
: filepath(filepath), entryPoint(entryPoint) {
	// Automatically add .spv if not present at the end of the filepath
	if (!std::regex_match(filepath, std::regex(R"(\.spv$)"))) {
		filepath += ".spv";
	}

	// Validate filepath
	std::regex filepathRegex{R"(^(.*/|)([^/]+)\.([^\.]+)(\.spv)$)"};
	if (!std::regex_match(filepath, filepathRegex)) {
		throw std::runtime_error("Invalid shader file path '" + filepath + "'");
	}

	// Read the file
	std::ifstream file(filepath, std::fstream::ate | std::fstream::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to load shader file '" + filepath + "'");
	}

	// Parse the file
	name = std::regex_replace(filepath, filepathRegex, "$2");
	type = std::regex_replace(filepath, filepathRegex, "$3");
	auto stageFlagBits = SHADER_TYPES[type];
	if (!stageFlagBits) {
		throw std::runtime_error("Invalid Shader Type " + type);
	}
	size_t fileSize = (size_t) file.tellg();
	bytecode.resize(fileSize);
	file.seekg(0);
	file.read(bytecode.data(), fileSize);
	file.close();
}

VkShaderModule Shader::CreateShaderModule(Device* device, VkPipelineShaderStageCreateFlags flags) {
	// Create the shaderModule
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytecode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());
	if (device->CreateShaderModule(&createInfo, nullptr, &module) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Shader Module for shader " + name);
	}
	
	// Specialization
	specialization.specializationInfo.mapEntryCount = specialization.specializationMap.size();
	specialization.specializationInfo.pMapEntries = specialization.specializationMap.data();
	specialization.specializationInfo.dataSize = specialization.specializationData.size();
	specialization.specializationInfo.pData = specialization.specializationData.data();

	// Create Stage Info
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.pNext = nullptr;
	stageInfo.flags = flags;
	stageInfo.stage = SHADER_TYPES[type];
	stageInfo.module = module;
	stageInfo.pName = entryPoint.c_str();
	stageInfo.pSpecializationInfo = specialization.specializationInfo.dataSize>0? &specialization.specializationInfo : nullptr;
	
	return module;
}

void Shader::DestroyShaderModule(Device* device) {
	device->DestroyShaderModule(module, nullptr);
}

ShaderInfo::ShaderInfo(const std::string& filepath, const std::string& entryPoint) 
: filepath(filepath), entryPoint(entryPoint) {}

ShaderInfo::ShaderInfo(const std::string& filepath)
: filepath(filepath), entryPoint("main") {}

ShaderInfo::ShaderInfo(const char* filepath)
: filepath(filepath), entryPoint("main") {}
