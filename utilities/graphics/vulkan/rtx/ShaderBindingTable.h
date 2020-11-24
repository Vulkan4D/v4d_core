#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan::rtx {
	using namespace v4d::graphics::vulkan;
	
	class V4DLIB ShaderBindingTable {
	private:
		
		std::map<uint32_t, ShaderInfo> shaderFiles;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayGenGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayMissGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayHitGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
		std::vector<Shader> shaderObjects;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		// uint32_t hitGroupOffset = 0;
		// uint32_t missGroupOffset = 0;
		
		uint32_t nextHitShaderOffset = 0;
		uint32_t nextMissShaderOffset = 0;
		
		VkDeviceSize bufferOffset = 0;
		VkDeviceSize bufferSize = 0;
		VkDeviceSize rayGenShaderRegionOffset = 0;
		VkDeviceSize rayGenShaderRegionSize = 0;
		VkDeviceSize rayMissShaderRegionOffset = 0;
		VkDeviceSize rayMissShaderRegionSize = 0;
		VkDeviceSize rayHitShaderRegionOffset = 0;
		VkDeviceSize rayHitShaderRegionSize = 0;
		VkStridedDeviceAddressRegionKHR rayGenDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayMissDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayHitDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayCallableDeviceAddressRegion {};
		
		PipelineLayout* pipelineLayout = nullptr;
		VkPipeline pipeline = VK_NULL_HANDLE;
		
	public:

		VkPipeline GetPipeline() const;
		PipelineLayout* GetPipelineLayout() const;
		// std::vector<VkRayTracingShaderGroupCreateInfoKHR> GetGroups() const;
		std::vector<VkPipelineShaderStageCreateInfo> GetStages() const;
		
		// uint32_t GetHitGroupOffset() const;
		// uint32_t GetMissGroupOffset() const;
		
		VkStridedDeviceAddressRegionKHR* GetRayGenDeviceAddressRegion();
		VkStridedDeviceAddressRegionKHR* GetRayMissDeviceAddressRegion();
		VkStridedDeviceAddressRegionKHR* GetRayHitDeviceAddressRegion();
		VkStridedDeviceAddressRegionKHR* GetRayCallableDeviceAddressRegion();

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
		
		void CreateShaderStages(Device*);
		void DestroyShaderStages(Device*);
		
		VkPipeline CreateRayTracingPipeline(Device*);
		void DestroyRayTracingPipeline(Device*);
		
		VkDeviceSize GetSbtBufferSize(const VkPhysicalDeviceRayTracingPipelinePropertiesKHR&);
		void WriteShaderBindingTableToBuffer(Device*, Buffer*, VkDeviceSize offset, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR&);
	};
}
