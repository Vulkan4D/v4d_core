#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan::rtx {
	using namespace v4d::graphics::vulkan;
	
	class V4DLIB ShaderBindingTable {
	private:
		
		std::map<uint32_t, ShaderInfo> shaderFiles;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
		std::vector<Shader> shaderObjects;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		uint32_t hitGroupOffset = 0;
		uint32_t missGroupOffset = 0;
		
		uint32_t nextHitShaderOffset = 0;
		uint32_t nextMissShaderOffset = 0;
		
		PipelineLayout* pipelineLayout = nullptr;
		VkPipeline pipeline = VK_NULL_HANDLE;
		
	public:

		VkPipeline GetPipeline() const;
		PipelineLayout* GetPipelineLayout() const;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> GetGroups() const;
		std::vector<VkPipelineShaderStageCreateInfo> GetStages() const;
		
		uint32_t GetHitGroupOffset() const;
		uint32_t GetMissGroupOffset() const;

		// Rules: 
			// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then generalShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_SHADER_STAGE_MISS_BIT_KHR, or VK_SHADER_STAGE_CALLABLE_BIT_KHR
			// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR then closestHitShader, anyHitShader, and intersectionShader must be VK_SHADER_UNUSED_KHR
			// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR then intersectionShader must be a valid index into pStages referring to a shader of VK_SHADER_STAGE_INTERSECTION_BIT_KHR
			// If type is VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR then intersectionShader must be VK_SHADER_UNUSED_KHR
			// closestHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
			// anyHitShader must be either VK_SHADER_UNUSED_KHR or a valid index into pStages referring to a shader of VK_SHADER_STAGE_ANY_HIT_BIT_KHR
		
		uint32_t GetOrAddShaderFileIndex(ShaderInfo&& shader);
		
		ShaderBindingTable(PipelineLayout& pipelineLayout, ShaderInfo rgen);
		
		uint32_t AddMissShader(ShaderInfo rmiss);
		uint32_t AddHitShader(ShaderInfo rchit, ShaderInfo rahit = "", ShaderInfo rint = "");
		
		void ReadShaders();
		
		void CreateShaderStages(Device* device);
		void DestroyShaderStages(Device* device);
		
		VkPipeline CreateRayTracingPipeline(Device* device);
		void DestroyRayTracingPipeline(Device* device);
		
		void WriteShaderBindingTableToBuffer(Device* device, Buffer* buffer, uint32_t shaderGroupHandleSize);
	};
}
