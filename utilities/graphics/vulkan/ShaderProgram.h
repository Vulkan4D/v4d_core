#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB ShaderProgram {
	private:
		std::vector<ShaderInfo> shaderFiles;
		std::vector<Shader> shaders;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		
		PipelineLayout* pipelineLayout = nullptr;
		
	public:

		ShaderProgram(PipelineLayout* pipelineLayout = nullptr, const std::vector<ShaderInfo>& infos = {});
		
		void AddVertexInputBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs);
		void AddVertexInputBinding(uint32_t stride, VkVertexInputRate inputRate, std::vector<VertexInputAttributeDescription> attrs);

		void LoadShaders();
		void UnloadShaders();
		
		void CreateShaderStages(Device* device);
		void DestroyShaderStages(Device* device);
		
		void SetPipelineLayout(PipelineLayout* layout);
		PipelineLayout* GetPipelineLayout() const;
		
		std::vector<VkPipelineShaderStageCreateInfo>* GetStages();
		std::vector<VkVertexInputBindingDescription>* GetBindings();
		std::vector<VkVertexInputAttributeDescription>* GetAttributes();
		
	};
}
