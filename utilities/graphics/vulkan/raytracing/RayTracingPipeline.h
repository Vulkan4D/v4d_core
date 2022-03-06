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
	
	class V4DLIB RayTracingPipeline {
	protected:
		
		ShaderPipelineMetaFile shaderPipelineMetaFile;
		std::map<uint32_t, ShaderInfo> shaderFiles;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayGenGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayMissGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayCallableGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> pipelineGroups; // rgen, rmiss, rcall
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayHitGroups;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> allGroups;
		std::vector<Shader> shaderObjects;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		
		uint32_t nextHitShaderOffset = 0;
		uint32_t nextMissShaderOffset = 0;
		uint32_t nextCallableShaderOffset = 0;
		
		VkDeviceSize pipelineBufferSize = 0;
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
		std::unique_ptr<StagingBuffer<uint8_t>> pipelineBuffer = nullptr;
		bool dirty = false;
		
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties {};
		VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties {};
		
		void ReadShaders();
		void CreateShaderStages();
		void DestroyShaderStages();
		
		VkPipeline CreateRayTracingPipeline();
		void DestroyRayTracingPipeline();
		
		VkDeviceSize GetSbtPipelineBufferSize();
		VkDeviceSize GetSbtBufferSize();
		void WritePipelineToSBT();
		
		struct RayTracingShaderProgram {
			ShaderInfo rchit {""};
			ShaderInfo rahit {""};
			ShaderInfo rint {""};
		};
		using RayTracingShaderPrograms = std::map<int/*subpass/offset*/, RayTracingShaderProgram>;

	public:

		template<template<class> class BufferContainerType, class MappedType>
		void WriteHitGroupsToSBT(BufferContainerType<MappedType>& buffer, const std::vector<uint32_t>& hitGroupsUsed, bool mayReallocate = false) {
			if (buffer.Count() < hitGroupsUsed.size()) {
				assert(mayReallocate);
				buffer.Resize(hitGroupsUsed.size() * 2, true);
			}
			
			if (hitGroupsDirty) {
				Reload();
				hitGroupsDirty = false;
			}
			
			const VkDeviceAddress& addr = buffer;
			const size_t& stride = buffer.TypeSize;
			const size_t count = hitGroupsUsed.size();
			const size_t size = count * stride;
			
			// Check Size & Alignment
			const uint32_t& handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
			const uint64_t& alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
			assert(sizeof(MappedType::sbtHandle) >= handleSize);
			assert(stride >= handleSize);
			assert(stride >= alignment);
			assert(buffer.Count() >= count);
			if (alignment > 0) {
				assert(stride % alignment == 0);
				assert(addr % alignment == 0);
			}
			
			// Region
			rayHitDeviceAddressRegion = {addr, stride, size};
			
			// Fill
			assert(rayHitGroups.size() > 0);
			const size_t shaderHandlerStorageSize = rayHitGroups.size()*handleSize;
			auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
			if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, (uint)pipelineGroups.size(), (uint)rayHitGroups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
				throw std::runtime_error("Failed to get ray tracing shader group handles");
			for (size_t i = 0; i < hitGroupsUsed.size(); ++i) {
				memcpy(buffer[i].sbtHandle, shaderHandleStorage + hitGroupsUsed[i]*handleSize, handleSize);
			}
			delete[] shaderHandleStorage;
		}
		
		void WriteSingleHitGroupHandle(void* dstBuffer, uint32_t sbtHitGroup) {
			assert(dstBuffer);
			if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, sbtHitGroup, 1, rayTracingPipelineProperties.shaderGroupHandleSize, dstBuffer) != VK_SUCCESS)
				throw std::runtime_error("Failed to get ray tracing shader group handle");
		}
		
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
		
		RayTracingPipeline(PipelineLayoutObject* pipelineLayout, const char* shaderFile);
		
		uint32_t AddMissShader(const ShaderInfo& rmiss);
		uint32_t AddHitShader(const char* filePath);
		uint32_t AddHitShader(const ShaderInfo& rchit, const ShaderInfo& rahit, const ShaderInfo& rint);
		uint32_t AddHitShaders(const RayTracingShaderPrograms&);
		uint32_t AddCallableShader(const ShaderInfo& rcall);
		
		// This one can be dynamic
		uint32_t GetOrAddHitGroup(const char* filePath);
		struct SharedHitGroup {
			uint32_t index;
			double modifiedTime;
			SharedHitGroup(uint32_t index)
			 : index(index), modifiedTime(0)
			{}
		};
		std::mutex sharedHitGroupsMutex;
		std::unordered_map<std::string, SharedHitGroup> sharedHitGroups {};
		bool hitGroupsDirty = false;
		
		void Configure(v4d::graphics::Renderer*, Device*);
		void Create(Device* device);
		void Push(VkCommandBuffer);
		void PushSBT(VkCommandBuffer, StagingBuffer<uint8_t>&);
		void Reload();
		void Destroy();
	};
}
