#pragma once

#include <v4d.h>

#ifndef RAY_TRACING_TLAS_MAX_INSTANCES
	#define RAY_TRACING_TLAS_MAX_INSTANCES 65536
#endif

namespace v4d::graphics {
	class Geometry;
}

namespace v4d::graphics::vulkan::rtx {
	
	struct V4DLIB RayTracingBLASInstance { // VkAccelerationStructureInstanceKHR
		glm::mat3x4 transform;
		uint32_t customInstanceId : 24;
		uint32_t mask : 8;
		uint32_t shaderInstanceOffset : 24;
		VkGeometryInstanceFlagsKHR flags : 8;
		uint64_t accelerationStructureHandle;
	};

	struct V4DLIB AccelerationStructure {
		// Fixed Properties
		bool isTopLevel = false;
		bool allowUpdate = false;
		
		// Handles
		VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		uint64_t handle = 0;
		
		// Structs
		VkAccelerationStructureCreateInfoKHR createInfo {};
		VkAccelerationStructureGeometryKHR* geometry = nullptr;
		VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo {};
		VkAccelerationStructureCreateGeometryTypeInfoKHR createGeometryTypeInfo {};
		VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo {};
		
		// State
		bool built = false;
		bool update = false;
		
		Device* device = nullptr;
		
		// Individual Scratch Buffer
		Buffer scratchBuffer {VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
		bool scratchBufferAllocated = false;
		
		// Global scratch buffer
		static bool useGlobalScratchBuffer;
		uint64_t globalScratchBufferOffset = 0;
		
		VkDeviceSize GetMemoryRequirementSizeForScratchBuffer(Device* device) const;
		
		void AssignBottomLevel(Device* device, std::shared_ptr<Geometry> geom);
		
		void AssignTopLevel();
		void SetGlobalScratchBuffer(Device* device, VkBuffer buffer);
		void SetInstanceBuffer(Device* device, VkBuffer instanceBuffer, uint32_t instanceCount = 0, uint32_t instanceOffset = 0);
		void SetInstanceBuffer(Device* device, void* instanceArray, uint32_t instanceCount = 0, uint32_t instanceOffset = 0);
		void SetInstanceCount(uint32_t count);
		
		~AccelerationStructure();
		
		void Create(Device* device, bool topLevel = false);
		void Destroy(Device* device);
		
		void Allocate(Device* device);
		void Free(Device* device);
	};
}
