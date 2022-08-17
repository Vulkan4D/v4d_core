#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Buffer.h"

#ifndef RAY_TRACING_TLAS_MAX_INSTANCES
	#define RAY_TRACING_TLAS_MAX_INSTANCES 1'048'576
#endif

#define VK_SHADER_STAGE_ALL_RAY_TRACING VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR

namespace v4d::graphics::vulkan::raytracing {
	
	struct V4DLIB RayTracingASInstance { // VkAccelerationStructureInstanceKHR
		glm::mat3x4 transform;
		uint32_t instanceCustomIndex : 24;
		uint32_t mask : 8;
		uint32_t instanceShaderBindingTableRecordOffset : 24;
		VkGeometryInstanceFlagsKHR flags : 8;
		VkDeviceAddress accelerationStructureReference;
	};

	struct V4DLIB AccelerationStructure {
		// Fixed Properties
		bool isTopLevel = false;
		bool allowUpdate = false;
		
		// Handles
		VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
		MemoryAllocation accelerationStructureAllocation = VK_NULL_HANDLE;
		VkBuffer accelerationStructureBuffer = VK_NULL_HANDLE; //TODO use a global buffer
		VkDeviceSize accelerationStructureSize = 0;
		VkDeviceSize accelerationStructureOffset = 0;
		VkDeviceAddress deviceAddress = 0;
		
		// Structs
		VkAccelerationStructureCreateInfoKHR createInfo {};
		std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometries {};
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRangeInfo {};
		VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo {};
		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo {};
		
		// State
		bool built = false;
		bool update = false;
		bool dirty = false;
		std::vector<uint32_t> maxPrimitiveCounts {};
		
		Device* device = nullptr;
		
		// Scratch Buffer
		std::unique_ptr<Buffer> scratchBuffer = nullptr;
		
		struct GeometryAccelerationStructureInfo {
			VkGeometryFlagsKHR flags = 0; // VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR
			VkDeviceOrHostAddressConstKHR indexBuffer {};
			size_t indexOffset = 0;
			size_t indexCount = 0;
			size_t indexStride = 0;
			VkDeviceOrHostAddressConstKHR vertexBuffer {};
			size_t vertexOffset = 0;
			size_t vertexCount = 0;
			size_t vertexStride = 0;
			VkDeviceOrHostAddressConstKHR transformBuffer {};
			size_t transformOffset = 0;
		};
		
		void AssignBottomLevelGeometry(const std::vector<GeometryAccelerationStructureInfo>& geometries);
		void AssignBottomLevelAabb(const std::vector<GeometryAccelerationStructureInfo>& geometries);
		
		void AssignTopLevel();
		
		void SetInstanceBuffer(VkDeviceOrHostAddressConstKHR instanceBufferAddr, uint32_t instanceCount = 0, uint32_t instanceOffset = 0);
		void SetInstanceCount(uint32_t count);
		
		~AccelerationStructure();
		
		void CreateAndAllocate(Device* device, bool topLevel = false);
		void FreeAndDestroy();
		
		VkDeviceSize AssignScratchBuffer(VkDeviceOrHostAddressConstKHR, VkDeviceSize offset = 0);
		void AllocateScratchBuffer();
		void FreeScratchBuffer();
		VkDeviceSize GetScratchBufferSizeRequirement() const {return accelerationStructureBuildSizesInfo.buildScratchSize;}
		
		const VkAccelerationStructureBuildGeometryInfoKHR& GetBuildGeometryInfo() {
			return buildGeometryInfo;
		}
		
		bool IsAllocated() const {return device != nullptr;}
	};
}
