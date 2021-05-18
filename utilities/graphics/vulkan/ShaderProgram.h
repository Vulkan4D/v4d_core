/*
 * Vulkan Shader Program abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class handles the loading of multiple shader stages into one shader 'program'
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/Shader.h"

namespace v4d::graphics::vulkan {

	class V4DLIB ShaderProgram {
	protected:
		std::vector<ShaderInfo> shaderFiles;
		std::vector<Shader> shaders;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		std::unordered_map<std::string, ShaderSpecialization> specializations {};
		
	public:

		ShaderProgram(const std::vector<ShaderInfo>& infos);
		virtual ~ShaderProgram();
		
		// reads the spv files and instantiates all Shaders in the shaders vector
		void ReadShaders();
		
		void CreateShaderStages(Device* device);
		void DestroyShaderStages(Device* device);
		
		template<typename T>
		void SetConstantValue(const std::string& stage, uint32_t id, const T& value) {
			specializations[stage].SetValue(id, value);
		}
		
		const std::vector<ShaderInfo>& GetShaderFiles() const {
			return shaderFiles;
		}
		std::vector<VkPipelineShaderStageCreateInfo>* GetStages() {
			return &stages;
		}
		
	};
	
}
