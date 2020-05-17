/*
 * Vulkan Shader Program abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class handles the loading of multiple shader stages into one shader 'program'
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB VertexInputAttributeDescription {
		uint32_t location;
		uint32_t offset;
		VkFormat format;
	};

	class V4DLIB ShaderProgram {
	private:
		std::vector<ShaderInfo> shaderFiles;
		std::vector<Shader> shaders;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		
		PipelineLayout* pipelineLayout = nullptr;
		
	public:

		ShaderProgram(PipelineLayout& pipelineLayout, const std::vector<ShaderInfo>& infos, int sortIndex = 0);
		virtual ~ShaderProgram();
		
		int sortIndex = 0;
		
		// sets up bindings
		void AddVertexInputBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs);
		void AddVertexInputBinding(uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs);

		// reads the spv files and instantiates all Shaders in the shaders vector
		void ReadShaders();
		
		// clears bindings and attributes
		void Reset();
		
		void CreateShaderStages(Device* device);
		void DestroyShaderStages(Device* device);
		
		void SetPipelineLayout(PipelineLayout* layout);
		PipelineLayout* GetPipelineLayout() const;
		
		std::vector<VkPipelineShaderStageCreateInfo>* GetStages();
		std::vector<VkVertexInputBindingDescription>* GetBindings();
		std::vector<VkVertexInputAttributeDescription>* GetAttributes();
		
	};
	
}
