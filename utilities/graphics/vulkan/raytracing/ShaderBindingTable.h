#pragma once

#include <v4d.h>
#include <vector>
#include <map>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Shader.h"
#include "utilities/graphics/vulkan/PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/BufferObject.h"
#include "utilities/graphics/Renderer.h"

namespace v4d::graphics::vulkan::raytracing {
	using namespace v4d::graphics::vulkan;
	
	class V4DLIB ShaderBindingTable {
	private:
		
		ShaderPipelineMetaFile shaderPipelineMetaFile;
		std::map<uint32_t, ShaderInfo> shaderFiles;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayGenGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayMissGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayHitGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayCallableGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
		std::vector<Shader> shaderObjects;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		uint32_t nextHitShaderOffset = 0;
		uint32_t nextMissShaderOffset = 0;
		uint32_t nextCallableShaderOffset = 0;
		
		VkDeviceSize bufferSize = 0;
		VkDeviceSize rayGenShaderRegionOffset = 0;
		VkDeviceSize rayGenShaderRegionSize = 0;
		VkDeviceSize rayMissShaderRegionOffset = 0;
		VkDeviceSize rayMissShaderRegionSize = 0;
		VkDeviceSize rayHitShaderRegionOffset = 0;
		VkDeviceSize rayHitShaderRegionSize = 0;
		VkDeviceSize rayCallableShaderRegionOffset = 0;
		VkDeviceSize rayCallableShaderRegionSize = 0;
		VkStridedDeviceAddressRegionKHR rayGenDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayMissDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayHitDeviceAddressRegion {};
		VkStridedDeviceAddressRegionKHR rayCallableDeviceAddressRegion {};
		
		PipelineLayoutObject* pipelineLayout = nullptr;
		VkPipeline pipeline = VK_NULL_HANDLE;
		
		Device* device = nullptr;
		std::unique_ptr<StagingBuffer<uint8_t>> buffer = nullptr;
		bool dirty = false;
		
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties {};
		VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties {};
		
		void ReadShaders();
		void CreateShaderStages();
		void DestroyShaderStages();
		
		VkPipeline CreateRayTracingPipeline();
		void DestroyRayTracingPipeline();
		
		VkDeviceSize GetSbtBufferSize();
		void WriteShaderBindingTableToBuffer();
		
		struct RayTracingShaderProgram {
			ShaderInfo rchit {""};
			ShaderInfo rahit {""};
			ShaderInfo rint {""};
		};
		using RayTracingShaderPrograms = std::map<int/*subpass/offset*/, RayTracingShaderProgram>;

	public:

		operator ShaderPipelineMetaFile() {
			return shaderPipelineMetaFile;
		}
		
		VkPipeline GetPipeline() const;
		PipelineLayoutObject* GetPipelineLayout() const;
		std::vector<VkPipelineShaderStageCreateInfo> GetStages() const;
		
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
		
		void PushConstant(VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex = 0);
		
		uint32_t GetOrAddShaderFileIndex(const ShaderInfo& shader);
		
		ShaderBindingTable(PipelineLayoutObject* pipelineLayout, const char* shaderFile);
		
		uint32_t AddMissShader(const ShaderInfo& rmiss);
		uint32_t AddHitShader(const char* filePath);
		uint32_t AddHitShader(const ShaderInfo& rchit, const ShaderInfo& rahit, const ShaderInfo& rint);
		uint32_t AddHitShaders(const RayTracingShaderPrograms&);
		uint32_t AddCallableShader(const ShaderInfo& rcall);
		
		void Configure(v4d::graphics::Renderer*, Device*);
		void Create(Device* device);
		void Push(VkCommandBuffer);
		void Reload();
		void Destroy();
	};
}
