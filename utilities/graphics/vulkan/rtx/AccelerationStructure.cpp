#include <v4d.h>

namespace v4d::graphics::vulkan::rtx {
	
	bool AccelerationStructure::useGlobalScratchBuffer = false;

	void AccelerationStructure::AssignBottomLevel(Device* device, std::shared_ptr<v4d::scene::Geometry> geom) {
		isTopLevel = false;
		this->device = device;
		
		if (!geometry) geometry = new VkAccelerationStructureGeometryKHR {};
		buildGeometryInfo.pGeometries = geometry;
		buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildGeometryInfo.geometryCount = 1;
		
		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			auto vertexBufferAddr = device->GetBufferDeviceOrHostAddressConst(v4d::scene::Geometry::globalBuffers.vertexBuffer.deviceLocalBuffer.buffer);
		#else
			auto vertexBufferAddr = device->GetBufferDeviceOrHostAddressConst(v4d::scene::Geometry::globalBuffers.vertexBuffer.buffer);
		#endif
		if (geom->isProcedural) {
			geometry->geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
			geometry->geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
			geometry->geometry.aabbs.stride = sizeof(v4d::scene::Geometry::ProceduralVertexBuffer_T);
			geometry->geometry.aabbs.data = vertexBufferAddr;
		} else {
			geometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			geometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			geometry->geometry.triangles.vertexStride = sizeof(v4d::scene::Geometry::VertexBuffer_T);
			geometry->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			geometry->geometry.triangles.indexType = sizeof(v4d::scene::Geometry::IndexBuffer_T)==2? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
			geometry->geometry.triangles.vertexData = vertexBufferAddr;
			geometry->geometry.triangles.maxVertex = geom->vertexCount;
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				geometry->geometry.triangles.indexData = device->GetBufferDeviceOrHostAddressConst(v4d::scene::Geometry::globalBuffers.indexBuffer.deviceLocalBuffer.buffer);
			#else
				geometry->geometry.triangles.indexData = device->GetBufferDeviceOrHostAddressConst(v4d::scene::Geometry::globalBuffers.indexBuffer.buffer);
			#endif
			if (geom->transformBuffer) {
				geometry->geometry.triangles.transformData = device->GetBufferDeviceOrHostAddressConst(geom->transformBuffer->buffer);
			}
		}
		
		buildRangeInfo = {
			geom->isProcedural? geom->vertexCount : (geom->indexCount/3), // primitiveCount
			(uint32_t)(geom->isProcedural? (geom->vertexOffset*sizeof(v4d::scene::Geometry::ProceduralVertexBuffer_T)) : (geom->indexOffset*sizeof(v4d::scene::Geometry::IndexBuffer_T))), // primitiveOffset
			geom->isProcedural? 0 : geom->vertexOffset, // firstVertex
			(uint32_t)geom->transformOffset // transformOffset
		};
		
		maxPrimitiveCount = buildRangeInfo.primitiveCount;
	}
	
	void AccelerationStructure::AssignTopLevel() {
		isTopLevel = true;
		
		if (!geometry) geometry = new VkAccelerationStructureGeometryKHR {};
		buildGeometryInfo.pGeometries = geometry;
		buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildGeometryInfo.geometryCount = 1;
		
		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry->geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry->geometry.instances.arrayOfPointers = VK_FALSE;
		
		maxPrimitiveCount = RAY_TRACING_TLAS_MAX_INSTANCES;
	}
	
	void AccelerationStructure::SetInstanceBuffer(Device* device, void* instanceArray, uint32_t instanceCount, uint32_t instanceOffset) {
		this->device = device;
		VkDeviceOrHostAddressConstKHR addr {};
		addr.hostAddress = instanceArray;
		geometry->geometry.instances.data = addr;
		buildRangeInfo.primitiveCount = instanceCount;
		buildRangeInfo.primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceBuffer(Device* device, VkBuffer instanceBuffer, uint32_t instanceCount, uint32_t instanceOffset) {
		this->device = device;
		geometry->geometry.instances.data = device->GetBufferDeviceOrHostAddressConst(instanceBuffer);
		buildRangeInfo.primitiveCount = instanceCount;
		buildRangeInfo.primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceCount(uint32_t count) {
		buildRangeInfo.primitiveCount = count;
	}
	
	void AccelerationStructure::SetGlobalScratchBuffer(Device* device, VkBuffer buffer) {
		buildGeometryInfo.scratchData = device->GetBufferDeviceOrHostAddress(buffer);
		buildGeometryInfo.scratchData.deviceAddress += globalScratchBufferOffset;
	}
	
	AccelerationStructure::~AccelerationStructure() {
		if (geometry) delete geometry;
		if (device) {
			FreeAndDestroy(device);
		}
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
				&maxPrimitiveCount,
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
		if (!useGlobalScratchBuffer && !scratchBufferAllocated) {
			scratchBuffer.size = accelerationStructureBuildSizesInfo.buildScratchSize;
			scratchBuffer.Allocate(device, MEMORY_USAGE_GPU_ONLY, false);
			scratchBufferAllocated = true;
			buildGeometryInfo.scratchData = device->GetBufferDeviceOrHostAddress(scratchBuffer.buffer);
		}
		
		{// Prep Build info
			buildGeometryInfo.srcAccelerationStructure = accelerationStructure;
			buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
		}
		
		// LOG_VERBOSE("Created Acceleration Structure " << accelerationStructure)
		
		if (!device->TouchAllocation(accelerationStructureAllocation)) {
			LOG_DEBUG("Acceleration Structure CreateAndAllocate ALLOCATION LOST")
		}
	}
	
	void AccelerationStructure::FreeAndDestroy(Device* device) {
		if (accelerationStructureAllocation) {
			device->FreeAndDestroyBuffer(accelerationStructureBuffer, accelerationStructureAllocation);
		}
		if (!useGlobalScratchBuffer && scratchBufferAllocated) {
			scratchBuffer.Free(device);
			scratchBufferAllocated = false;
		}
		if (accelerationStructure) {
			// LOG_VERBOSE("Destroyed Acceleration Structure " << accelerationStructure << "; handle " << std::hex << handle)
			device->DestroyAccelerationStructureKHR(accelerationStructure, nullptr);
			// LOG_VERBOSE("Destroyed Acceleration Structure " << accelerationStructure)
			accelerationStructure = VK_NULL_HANDLE;
		}
		deviceAddress = 0;
		built = false;
	}
}
