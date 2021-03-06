#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Buffer.h"

#ifndef RAY_TRACING_TLAS_MAX_INSTANCES
	#define RAY_TRACING_TLAS_MAX_INSTANCES 65536
#endif

namespace v4d::graphics::vulkan::rtx {
	
	struct V4DLIB RayTracingBLASInstance { // VkAccelerationStructureInstanceKHR
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
		std::vector<uint32_t> maxPrimitiveCounts {};
		
		Device* device = nullptr;
		
		// Individual Scratch Buffer
		Buffer scratchBuffer {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
		bool scratchBufferAllocated = false;
		
		// Global scratch buffer
		static bool useGlobalScratchBuffer;
		uint64_t globalScratchBufferOffset = 0;
		
		struct GeometryAccelerationStructureInfo {
			VkGeometryFlagsKHR flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
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
		
		void AssignBottomLevelGeometry(Device* device, const std::vector<GeometryAccelerationStructureInfo>& geometries);
		void AssignBottomLevelProceduralVertex(Device* device, const std::vector<GeometryAccelerationStructureInfo>& geometries);
		
		void AssignTopLevel();
		
		void SetGlobalScratchBuffer(Device* device, VkBuffer buffer);
		void SetInstanceBuffer(Device* device, VkBuffer instanceBuffer, uint32_t instanceCount = 0, uint32_t instanceOffset = 0);
		void SetInstanceBuffer(Device* device, void* instanceArray, uint32_t instanceCount = 0, uint32_t instanceOffset = 0);
		void SetInstanceCount(uint32_t count);
		
		~AccelerationStructure();
		
		void CreateAndAllocate(Device* device, bool topLevel = false);
		void FreeAndDestroy(Device* device);
	};
}
