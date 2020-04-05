#include <v4d.h>

namespace v4d::graphics::vulkan::rtx {
	
	bool AccelerationStructure::useGlobalScratchBuffer = false;

	VkDeviceSize AccelerationStructure::GetMemoryRequirementsForScratchBuffer(Device* device) const {
		VkMemoryRequirements2 memoryRequirementsBuild {};
			memoryRequirementsBuild.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		VkAccelerationStructureMemoryRequirementsInfoKHR memoryRequirementsInfo {};
			memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
			memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
			memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
			memoryRequirementsInfo.accelerationStructure = accelerationStructure;
		device->GetAccelerationStructureMemoryRequirementsKHR(&memoryRequirementsInfo, &memoryRequirementsBuild);
		VkDeviceSize size, alignment;
		if (allowUpdate) {
			VkMemoryRequirements2 memoryRequirementsUpdate {};
			memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
			device->GetAccelerationStructureMemoryRequirementsKHR(&memoryRequirementsInfo, &memoryRequirementsUpdate);
			size = std::max(memoryRequirementsBuild.memoryRequirements.size, memoryRequirementsUpdate.memoryRequirements.size);
			alignment = memoryRequirementsUpdate.memoryRequirements.alignment;
		} else {
			size = memoryRequirementsBuild.memoryRequirements.size;
			alignment = memoryRequirementsBuild.memoryRequirements.alignment;
		}
		if ((size % alignment) > 0) {
			size += alignment - (size % alignment);
		}
		return size;
	}
	
	void AccelerationStructure::AssignBottomLevel(Device* device, std::shared_ptr<Geometry> geom) {
		isTopLevel = false;
		this->device = device;
		
		if (!geometry) geometry = new VkAccelerationStructureGeometryKHR {};
		buildGeometryInfo.ppGeometries = &geometry;
		
		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			auto vertexBufferAddr = device->GetBufferDeviceOrHostAddressConst(Geometry::globalBuffers.vertexBuffer.deviceLocalBuffer.buffer);
		#else
			auto vertexBufferAddr = device->GetBufferDeviceOrHostAddressConst(Geometry::globalBuffers.vertexBuffer.buffer);
		#endif
		if (geom->isProcedural) {
			geometry->geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
			geometry->geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
			geometry->geometry.aabbs.stride = sizeof(Geometry::ProceduralVertexBuffer_T);
			geometry->geometry.aabbs.data = vertexBufferAddr;
		} else {
			geometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			geometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			geometry->geometry.triangles.vertexStride = sizeof(Geometry::VertexBuffer_T);
			geometry->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			geometry->geometry.triangles.indexType = sizeof(Geometry::IndexBuffer_T)==2? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
			geometry->geometry.triangles.vertexData = vertexBufferAddr;
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				geometry->geometry.triangles.indexData = device->GetBufferDeviceOrHostAddressConst(Geometry::globalBuffers.indexBuffer.deviceLocalBuffer.buffer);
			#else
				geometry->geometry.triangles.indexData = device->GetBufferDeviceOrHostAddressConst(Geometry::globalBuffers.indexBuffer.buffer);
			#endif
			if (geom->transformBuffer) {
				geometry->geometry.triangles.transformData = device->GetBufferDeviceOrHostAddressConst(geom->transformBuffer->buffer);
			}
		}
		createGeometryTypeInfo = {
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
			nullptr,
			geometry->geometryType,
			geom->isProcedural? geom->vertexCount : (geom->indexCount/3),// maxPrimitiveCount
			geom->isProcedural? VK_INDEX_TYPE_NONE_KHR : geometry->geometry.triangles.indexType, // indexType
			geom->isProcedural? 0 : geom->vertexCount, // maxVertexCount
			geom->isProcedural? VK_FORMAT_UNDEFINED : geometry->geometry.triangles.vertexFormat, // vertexFormat
			(VkBool32)(geom->transformBuffer? VK_TRUE : VK_FALSE)
		};
		buildOffsetInfo = {
			geom->isProcedural? geom->vertexCount : (geom->indexCount/3), // primitiveCount
			(uint32_t)(geom->isProcedural? (geom->vertexOffset*sizeof(Geometry::ProceduralVertexBuffer_T)) : (geom->indexOffset*sizeof(Geometry::IndexBuffer_T))), // primitiveOffset
			geom->isProcedural? 0 : geom->vertexOffset, // firstVertex
			(uint32_t)geom->transformOffset // transformOffset
		};
	}
	
	void AccelerationStructure::AssignTopLevel() {
		isTopLevel = true;
		
		if (!geometry) geometry = new VkAccelerationStructureGeometryKHR {};
		buildGeometryInfo.ppGeometries = &geometry;
		
		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry->geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry->geometry.instances.arrayOfPointers = VK_FALSE;
		
		createGeometryTypeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		createGeometryTypeInfo.pNext = nullptr;
		createGeometryTypeInfo.maxPrimitiveCount = RAY_TRACING_TLAS_MAX_INSTANCES;
		createGeometryTypeInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		
		// unused
			createGeometryTypeInfo.maxVertexCount = 0;
			createGeometryTypeInfo.indexType = VK_INDEX_TYPE_NONE_KHR;
			createGeometryTypeInfo.vertexFormat = VK_FORMAT_UNDEFINED;
			createGeometryTypeInfo.allowsTransforms = VK_TRUE;
			buildOffsetInfo.firstVertex = 0;
			buildOffsetInfo.transformOffset = 0;
	}
	void AccelerationStructure::SetInstanceBuffer(Device* device, void* instanceArray, uint32_t instanceCount, uint32_t instanceOffset) {
		this->device = device;
		VkDeviceOrHostAddressConstKHR addr {};
		addr.hostAddress = instanceArray;
		geometry->geometry.instances.data = addr;
		buildOffsetInfo.primitiveCount = instanceCount;
		buildOffsetInfo.primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceBuffer(Device* device, VkBuffer instanceBuffer, uint32_t instanceCount, uint32_t instanceOffset) {
		this->device = device;
		geometry->geometry.instances.data = device->GetBufferDeviceOrHostAddressConst(instanceBuffer);
		buildOffsetInfo.primitiveCount = instanceCount;
		buildOffsetInfo.primitiveOffset = instanceOffset;
	}
	void AccelerationStructure::SetInstanceCount(uint32_t count) {
		buildOffsetInfo.primitiveCount = count;
	}
	
	void AccelerationStructure::SetGlobalScratchBuffer(Device* device, VkBuffer buffer) {
		buildGeometryInfo.scratchData = device->GetBufferDeviceOrHostAddress(buffer);
		buildGeometryInfo.scratchData.deviceAddress += globalScratchBufferOffset;
	}
	
	AccelerationStructure::~AccelerationStructure() {
		if (geometry) delete geometry;
		if (device) {
			Destroy(device);
			Free(device);
		}
	}
	
	void AccelerationStructure::Create(Device* device, bool topLevel) {
		isTopLevel = topLevel;
		this->device = device;
		
		uint32_t flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		if (allowUpdate) flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		
		createInfo = {
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			nullptr,// pNext
			0,// VkDeviceSize compactedSize
			isTopLevel? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			flags,
			1, // maxGeometryCount
			&createGeometryTypeInfo, // pGeometryInfos
			0 // VkDeviceAddress deviceAddress
		};
		
		if (device->CreateAccelerationStructureKHR(&createInfo, nullptr, &accelerationStructure) != VK_SUCCESS)
			throw std::runtime_error("Failed to create top level acceleration structure");
		
		// Build info
		buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildGeometryInfo.type = isTopLevel? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		if (allowUpdate) buildGeometryInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		buildGeometryInfo.update = (allowUpdate && built && update)? VK_TRUE : VK_FALSE;
		buildGeometryInfo.srcAccelerationStructure = accelerationStructure;
		buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
		buildGeometryInfo.geometryCount = 1;
		buildGeometryInfo.geometryArrayOfPointers = VK_FALSE;
		
		// LOG_VERBOSE("Created Acceleration Structure " << accelerationStructure)
	}
	
	void AccelerationStructure::Destroy(Device* device) {
		if (accelerationStructure) {
			device->DestroyAccelerationStructureKHR(accelerationStructure, nullptr);
			// LOG_VERBOSE("Destroyed Acceleration Structure " << accelerationStructure)
			accelerationStructure = VK_NULL_HANDLE;
		}
		built = false;
	}
	
	void AccelerationStructure::Allocate(Device* device) {
		// Allocate and bind memory for acceleration structure
		VkMemoryRequirements2 memoryRequirementsAS {};
		{
			memoryRequirementsAS.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
			VkAccelerationStructureMemoryRequirementsInfoKHR memoryRequirementsInfo {};
				memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
				memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
				memoryRequirementsInfo.accelerationStructure = accelerationStructure;
			device->GetAccelerationStructureMemoryRequirementsKHR(&memoryRequirementsInfo, &memoryRequirementsAS);
		}
		VkMemoryAllocateInfo memoryAllocateInfo {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,// pNext
			memoryRequirementsAS.memoryRequirements.size,// VkDeviceSize allocationSize
			device->GetPhysicalDevice()->FindMemoryType(memoryRequirementsAS.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)// memoryTypeIndex
		};
		if (device->AllocateMemory(&memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate memory for acceleration structure");
		VkBindAccelerationStructureMemoryInfoKHR accelerationStructureMemoryInfo {
			VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR,
			nullptr,// pNext
			accelerationStructure,// accelerationStructure
			memory,// memory
			0,// VkDeviceSize memoryOffset
			0,// uint32_t deviceIndexCount
			nullptr// pDeviceIndices
		};
		if (device->BindAccelerationStructureMemoryKHR(1, &accelerationStructureMemoryInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind acceleration structure memory");
			
		// Get acceleration structure handle for use in instances
		VkAccelerationStructureDeviceAddressInfoKHR devAddrInfo {};
			devAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			devAddrInfo.accelerationStructure = accelerationStructure;
		handle = device->GetAccelerationStructureDeviceAddressKHR(&devAddrInfo);
		
		// LOG_VERBOSE("Allocated Acceleration Structure " << accelerationStructure << "; handle " << handle << "; memory " << memory)
		
		// Scratch buffer
		if (!useGlobalScratchBuffer && !scratchBufferAllocated) {
			scratchBuffer.size = GetMemoryRequirementsForScratchBuffer(device);
			scratchBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
			scratchBufferAllocated = true;
			buildGeometryInfo.scratchData = device->GetBufferDeviceOrHostAddress(scratchBuffer.buffer);
		}
	}
	
	void AccelerationStructure::Free(Device* device) {
		if (memory) {
			device->FreeMemory(memory, nullptr);
			// LOG_VERBOSE("Freed Acceleration Structure " << accelerationStructure << "; handle " << handle << "; memory " << memory)
			memory = VK_NULL_HANDLE;
		}
		if (!useGlobalScratchBuffer && scratchBufferAllocated) {
			scratchBuffer.Free(device);
			scratchBufferAllocated = false;
		}
		handle = 0;
	}
}
