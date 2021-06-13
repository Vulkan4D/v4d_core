#include "AccelerationStructure.h"

namespace v4d::graphics::vulkan::raytracing {
	
	void AccelerationStructure::AssignBottomLevelGeometry(const std::vector<GeometryAccelerationStructureInfo>& geometries) {
		isTopLevel = false;
		
		accelerationStructureGeometries.clear();
		accelerationStructureGeometries.reserve(geometries.size());
		buildRangeInfo.clear();
		buildRangeInfo.reserve(geometries.size());
		maxPrimitiveCounts.clear();
		maxPrimitiveCounts.reserve(geometries.size());
		
		for (const auto& geom : geometries) {
			auto& accelerationStructureGeometry = accelerationStructureGeometries.emplace_back();
			auto& range = buildRangeInfo.emplace_back();
			
			accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			accelerationStructureGeometry.flags = geom.flags;

			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry = {};
			accelerationStructureGeometry.geometry.aabbs = {};
			accelerationStructureGeometry.geometry.instances = {};
			accelerationStructureGeometry.geometry.triangles = {};
			accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			accelerationStructureGeometry.geometry.triangles.vertexStride = geom.vertexStride;
			accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			accelerationStructureGeometry.geometry.triangles.vertexData = geom.vertexBuffer;
			accelerationStructureGeometry.geometry.triangles.maxVertex = geom.vertexCount;
			if (geom.transformBuffer.deviceAddress) {
				accelerationStructureGeometry.geometry.triangles.transformData = geom.transformBuffer;
			}
			range.transformOffset = (uint32_t)geom.transformOffset;
			
			// Indices
			switch (geom.indexStride) {
				case 0:
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
				break;
				case 2:
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT16;
				break;
				case 4:
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
				break;
				default: throw std::runtime_error("Unsupported index type");
			}
			if (geom.indexStride > 0) {
				accelerationStructureGeometry.geometry.triangles.indexData = geom.indexBuffer;
				range.primitiveCount = (uint32_t)geom.indexCount / 3;
				range.primitiveOffset = (uint32_t)geom.indexOffset;
				range.firstVertex = (uint32_t)(geom.vertexOffset / geom.vertexStride);
			} else {
				range.primitiveCount = (uint32_t)geom.vertexCount / 3;
				range.primitiveOffset = (uint32_t)geom.vertexOffset;
				range.firstVertex = 0;
			}
			
			maxPrimitiveCounts.emplace_back(range.primitiveCount);
		}
		
		buildGeometryInfo.pGeometries = accelerationStructureGeometries.data();
		buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildGeometryInfo.geometryCount = geometries.size();
	}
	
	void AccelerationStructure::AssignBottomLevelProceduralVertex(const std::vector<GeometryAccelerationStructureInfo>& geometries) {
		isTopLevel = false;
		
		accelerationStructureGeometries.clear();
		accelerationStructureGeometries.reserve(geometries.size());
		buildRangeInfo.clear();
		buildRangeInfo.reserve(geometries.size());
		maxPrimitiveCounts.clear();
		maxPrimitiveCounts.reserve(geometries.size());
		
		for (const auto& geom : geometries) {
			auto& accelerationStructureGeometry = accelerationStructureGeometries.emplace_back();
			auto& range = buildRangeInfo.emplace_back();
			
			accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			accelerationStructureGeometry.flags = geom.flags;
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
			accelerationStructureGeometry.geometry = {};
			accelerationStructureGeometry.geometry.aabbs = {};
			accelerationStructureGeometry.geometry.instances = {};
			accelerationStructureGeometry.geometry.triangles = {};
			accelerationStructureGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
			accelerationStructureGeometry.geometry.aabbs.stride = geom.vertexStride;
			accelerationStructureGeometry.geometry.aabbs.data = geom.vertexBuffer;
			
			range.primitiveCount = (uint32_t)geom.vertexCount;
			range.primitiveOffset = (uint32_t)(geom.vertexOffset * geom.vertexStride);
			range.firstVertex = 0;
			range.transformOffset = (uint32_t)geom.transformOffset;
			
			maxPrimitiveCounts.emplace_back(range.primitiveCount);
		}
		
		buildGeometryInfo.pGeometries = accelerationStructureGeometries.data();
		buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildGeometryInfo.geometryCount = geometries.size();
	}
	
	void AccelerationStructure::AssignTopLevel() {
		isTopLevel = true;
		
		accelerationStructureGeometries.clear();
		accelerationStructureGeometries.resize(1);
		buildRangeInfo.clear();
		buildRangeInfo.resize(1);
		maxPrimitiveCounts.resize(1);
		
		buildGeometryInfo.pGeometries = accelerationStructureGeometries.data();
		buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildGeometryInfo.geometryCount = 1;
		buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
		
		accelerationStructureGeometries[0].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometries[0].geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometries[0].geometry = {};
		accelerationStructureGeometries[0].geometry.aabbs = {};
		accelerationStructureGeometries[0].geometry.instances = {};
		accelerationStructureGeometries[0].geometry.triangles = {};
		accelerationStructureGeometries[0].geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometries[0].geometry.instances.arrayOfPointers = VK_FALSE;
		
		maxPrimitiveCounts[0] = RAY_TRACING_TLAS_MAX_INSTANCES;
	}
	
	void AccelerationStructure::SetInstanceBuffer(void* instanceArray, uint32_t instanceCount, uint32_t instanceOffset) {
		VkDeviceOrHostAddressConstKHR addr {};
		addr.hostAddress = instanceArray;
		accelerationStructureGeometries[0].geometry.instances.data = addr;
		buildRangeInfo[0].primitiveCount = instanceCount;
		buildRangeInfo[0].primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceBuffer(VkBuffer instanceBuffer, uint32_t instanceCount, uint32_t instanceOffset) {
		accelerationStructureGeometries[0].geometry.instances.data = device->GetBufferDeviceOrHostAddressConst(instanceBuffer);
		buildRangeInfo[0].primitiveCount = instanceCount;
		buildRangeInfo[0].primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceCount(uint32_t count) {
		buildRangeInfo[0].primitiveCount = count;
	}
	
	AccelerationStructure::~AccelerationStructure() {
		FreeAndDestroy();
	}
	
	void AccelerationStructure::CreateAndAllocate(Device* device, bool topLevel) {
		isTopLevel = topLevel;
		this->device = device;
		
		{// Prep Build info
			buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			if (allowUpdate) buildGeometryInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
			buildGeometryInfo.mode = (allowUpdate && built && update)? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		}
		
		{// Get build sizes
			accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			device->GetAccelerationStructureBuildSizesKHR(
				VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&buildGeometryInfo,
				maxPrimitiveCounts.data(),
				&accelerationStructureBuildSizesInfo);
			accelerationStructureSize = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		}
		
		{// Create and Allocate Buffer
			VkBufferCreateInfo bufferCreateInfo{};{
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.size = accelerationStructureSize;
				bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			}
			device->CreateAndAllocateBuffer(bufferCreateInfo, MEMORY_USAGE_GPU_ONLY, accelerationStructureBuffer, &accelerationStructureAllocation, true);
		}
		
		{// Create acceleration structure
			createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			createInfo.buffer = accelerationStructureBuffer;
			createInfo.size = accelerationStructureSize;
			createInfo.offset = accelerationStructureOffset;
			createInfo.type = isTopLevel? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			if (device->CreateAccelerationStructureKHR(&createInfo, nullptr, &accelerationStructure) != VK_SUCCESS)
				throw std::runtime_error("Failed to create top level acceleration structure");
		}
		
		// Get acceleration structure handle/deviceAddress for use in instances
		VkAccelerationStructureDeviceAddressInfoKHR devAddrInfo {};{
			devAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			devAddrInfo.accelerationStructure = accelerationStructure;
			deviceAddress = device->GetAccelerationStructureDeviceAddressKHR(&devAddrInfo);
		}
		
		// LOG_VERBOSE("Allocated Acceleration Structure " << accelerationStructure << "; deviceAddress " << std::hex << deviceAddress)
		
		// Scratch buffer
		scratchBuffer = std::make_unique<BufferObject>(MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, accelerationStructureBuildSizesInfo.buildScratchSize);
		scratchBuffer->Allocate(device);
		buildGeometryInfo.scratchData = VkDeviceOrHostAddressKHR(*scratchBuffer);
		
		{// Prep Build info
			buildGeometryInfo.srcAccelerationStructure = accelerationStructure;
			buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
		}
		
		// LOG_VERBOSE("Created Acceleration Structure " << accelerationStructure)
		
		if (!device->TouchAllocation(accelerationStructureAllocation)) {
			// LOG_DEBUG("Acceleration Structure CreateAndAllocate ALLOCATION LOST")
		}
	}
	
	void AccelerationStructure::FreeAndDestroy() {
		if (device) {
			if (accelerationStructureAllocation) {
				device->FreeAndDestroyBuffer(accelerationStructureBuffer, accelerationStructureAllocation);
			}
			scratchBuffer = nullptr;
			if (accelerationStructure) {
				// LOG_VERBOSE("Destroyed Acceleration Structure " << accelerationStructure << "; handle " << std::hex << handle)
				device->DestroyAccelerationStructureKHR(accelerationStructure, nullptr);
				// LOG_VERBOSE("Destroyed Acceleration Structure " << accelerationStructure)
				accelerationStructure = VK_NULL_HANDLE;
			}
			deviceAddress = 0;
			built = false;
			device = nullptr;
		}
	}
}
