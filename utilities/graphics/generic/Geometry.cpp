#include <v4d.h>

namespace v4d::graphics {
	
	Geometry::GlobalGeometryBuffers Geometry::globalBuffers {};

	VkGeometryNV Geometry::GetRayTracingGeometry() const {
		VkGeometryNV triangleGeometry {};
		triangleGeometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		triangleGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		triangleGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		triangleGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		triangleGeometry.flags = 0;//VK_GEOMETRY_OPAQUE_BIT_NV;
		
		triangleGeometry.geometry.triangles.vertexOffset = (VkDeviceSize)(vertexOffset*sizeof(PosBuffer_T));
		triangleGeometry.geometry.triangles.vertexCount = vertexCount;
		triangleGeometry.geometry.triangles.vertexStride = sizeof(PosBuffer_T);
		triangleGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangleGeometry.geometry.triangles.indexOffset = (VkDeviceSize)(indexOffset*sizeof(IndexBuffer_T));
		triangleGeometry.geometry.triangles.indexCount = indexCount;
		triangleGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		
		triangleGeometry.geometry.triangles.vertexData = globalBuffers.posBuffer.deviceLocalBuffer.buffer;
		triangleGeometry.geometry.triangles.indexData = globalBuffers.indexBuffer.deviceLocalBuffer.buffer;
		
		triangleGeometry.geometry.triangles.transformData = transformBuffer? transformBuffer->buffer : VK_NULL_HANDLE;
		triangleGeometry.geometry.triangles.transformOffset = transformOffset;
		
		return triangleGeometry;
	}
	
	Geometry::Geometry(uint32_t vertexCount, uint32_t indexCount) : vertexCount(vertexCount), indexCount(indexCount) {
		globalBuffers.AddGeometry(this);
	}
	
	Geometry::~Geometry() {
		globalBuffers.RemoveGeometry(this);
	}
	
	void Geometry::MapStagingBuffers() {
		if (!globalBuffers.geometryBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before mapping staging buffers")
		}
		geometryInfo =		&((GeometryBuffer_T*)	(globalBuffers.geometryBuffer.stagingBuffer.data))	[geometryOffset];
		indices =			&((IndexBuffer_T*)		(globalBuffers.indexBuffer.stagingBuffer.data))		[indexOffset];
		vertexPositions =	&((PosBuffer_T*)		(globalBuffers.posBuffer.stagingBuffer.data))		[vertexOffset];
		vertexNormals =		&((NormalBuffer_T*)		(globalBuffers.normalBuffer.stagingBuffer.data))	[vertexOffset];
		vertexColors =		&((ColorBuffer_T*)		(globalBuffers.colorBuffer.stagingBuffer.data))		[vertexOffset];
		
		// LOG("Global Geometry Staging Buffers mapped")
	}
	
	void Geometry::UnmapStagingBuffers() {
		geometryInfo = nullptr;
		indices = nullptr;
		vertexPositions = nullptr;
		vertexNormals = nullptr;
		vertexColors = nullptr;
		
		geometryInfoInitialized = false;
	}
	
	void Geometry::SetGeometryInfo(uint32_t objectIndex, uint32_t otherIndex) {
		if (!geometryInfo) MapStagingBuffers();
		
		GeometryBuffer_T data {
			indexOffset,
			vertexOffset,
			objectIndex,
			otherIndex
		};
		
		memcpy(geometryInfo, &data, sizeof(GeometryBuffer_T));
		
		geometryInfoInitialized = true;
	}
	
	void Geometry::SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color) {
		if (!vertexPositions) MapStagingBuffers();
		
		NormalBuffer_T packedNormal = PackNormal(normal);
		ColorBuffer_T packedColor = PackColor(color);
		PosBuffer_T vert {
			{pos, PackUV(uv)},
		};
		memcpy(&vertexPositions[i], &vert, sizeof(PosBuffer_T));
		memcpy(&vertexNormals[i], &packedNormal, sizeof(NormalBuffer_T));
		memcpy(&vertexColors[i], &packedColor, sizeof(ColorBuffer_T));
	}
	
	void Geometry::SetIndex(uint32_t i, IndexBuffer_T vertexIndex) {
		if (!indices) MapStagingBuffers();
		
		memcpy(&indices[i], &vertexIndex, sizeof(IndexBuffer_T));
	}
	
	void Geometry::SetIndices(const std::vector<IndexBuffer_T>& vertexIndices, uint32_t count, uint32_t startAt) {
		if (!indices) MapStagingBuffers();
		
		if (vertexIndices.size() == 0) return;
		
		if (count == 0) count = std::min(indexCount, (uint32_t)vertexIndices.size());
		else count = std::min(count, (uint32_t)vertexIndices.size());
		
		memcpy(&indices[startAt], vertexIndices.data(), sizeof(IndexBuffer_T)*count);
	}
	
	void Geometry::SetIndices(const IndexBuffer_T* vertexIndices, uint32_t count, uint32_t startAt) {
		if (!indices) MapStagingBuffers();
		
		if (count == 0) count = indexCount;
		else count = std::min(count, indexCount);
	
		memcpy(&indices[startAt], vertexIndices, sizeof(IndexBuffer_T)*count);
	}
	
	void Geometry::GetGeometryInfo(uint32_t* objectIndex, uint32_t* otherIndex) {
		if (!geometryInfo) MapStagingBuffers();
		
		GeometryBuffer_T data;
		memcpy(&data, &geometryInfo, sizeof(GeometryBuffer_T));
		indexOffset = data.x;
		vertexOffset = data.y;
		*objectIndex = data.z;
		*otherIndex = data.w;
	}
	
	void Geometry::GetVertex(uint32_t i, glm::vec3* pos, glm::vec3* normal, glm::vec2* uv, glm::vec4* color) {
		if (!vertexPositions) MapStagingBuffers();
		
		PosBuffer_T vert;
		NormalBuffer_T packedNormal;
		ColorBuffer_T packedColor;
		memcpy(&vert, &vertexPositions[i], sizeof(PosBuffer_T));
		memcpy(&packedNormal, &vertexNormals[i], sizeof(NormalBuffer_T));
		memcpy(&packedColor, &vertexColors[i], sizeof(ColorBuffer_T));
		*pos = vert.posAndUV;
		*uv = UnpackUV(vert.posAndUV.w);
		*normal = UnpackNormal(packedNormal);
		*color = UnpackColor(packedColor);
	}
	
	void Geometry::GetIndex(uint32_t i, IndexBuffer_T* vertexIndex) {
		if (!indices) MapStagingBuffers();
		
		memcpy(vertexIndex, &indices[i], sizeof(IndexBuffer_T));
	}
	
	void Geometry::GetIndices(std::vector<IndexBuffer_T>* vertexIndices, uint32_t count, uint32_t startAt) {
		if (!indices) MapStagingBuffers();
	
		if (vertexIndices) {
			if (count == 0) count = indexCount;
			if (vertexIndices->capacity() < count) vertexIndices->resize(count);
			memcpy(vertexIndices->data(), &indices[startAt], sizeof(IndexBuffer_T)*count);
		} else {
			LOG_ERROR("vertexIndices vector pointer arg must be allocated")
		}
	}
	
	void Geometry::Push(Device* device, VkCommandBuffer commandBuffer, 
			GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask, 
			uint32_t vertexCount, uint32_t vertexOffset,
			uint32_t indexCount, uint32_t indexOffset
		) {
		globalBuffers.PushGeometry(device, commandBuffer, this, geometryBuffersMask, vertexCount, vertexOffset, indexCount, indexOffset);
	}

	void Geometry::Pull(Device* device, VkCommandBuffer commandBuffer, 
			GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask, 
			uint32_t vertexCount, uint32_t vertexOffset,
			uint32_t indexCount, uint32_t indexOffset
		) {
		globalBuffers.PullGeometry(device, commandBuffer, this, geometryBuffersMask, vertexCount, vertexOffset, indexCount, indexOffset);
	}
	
	#pragma region GlobalGeometryBuffers
	
	int Geometry::GlobalGeometryBuffers::AddGeometry(Geometry* geometry) {
		std::scoped_lock lock(geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		
		// Geometry
		geometry->geometryOffset = nbAllocatedGeometries;
		for (auto [i, g] : geometryAllocations) {
			if (!g) {
				geometry->geometryOffset = i;
				break;
			}
		}
		nbAllocatedGeometries = std::max(nbAllocatedGeometries, geometry->geometryOffset + 1);
		geometryAllocations[geometry->geometryOffset] = geometry;
		
		// Index
		geometry->indexOffset = nbAllocatedIndices;
		for (auto [i, alloc] : indexAllocations) {
			if (!alloc.data) {
				if (alloc.n >= geometry->indexCount) {
					geometry->indexOffset = i;
					if (alloc.n > geometry->indexCount) {
						indexAllocations[geometry->indexOffset+geometry->indexCount] = {alloc.n - geometry->indexCount, nullptr};
					}
				}
			}
		}
		nbAllocatedIndices = std::max(nbAllocatedIndices, geometry->indexOffset + geometry->indexCount);
		indexAllocations[geometry->indexOffset] = {geometry->indexCount, geometry};
		
		// Vertex
		geometry->vertexOffset = nbAllocatedVertices;
		for (auto [i, alloc] : vertexAllocations) {
			if (!alloc.data) {
				if (alloc.n >= geometry->vertexCount) {
					geometry->vertexOffset = i;
					if (alloc.n > geometry->vertexCount) {
						vertexAllocations[geometry->vertexOffset+geometry->vertexCount] = {alloc.n - geometry->vertexCount, nullptr};
					}
				}
			}
		}
		nbAllocatedVertices = std::max(nbAllocatedVertices, geometry->vertexOffset + geometry->vertexCount);
		vertexAllocations[geometry->vertexOffset] = {geometry->vertexCount, geometry};
		
		// Check for overflow
		if (geometryBuffer.stagingBuffer.size < nbAllocatedGeometries * sizeof(GeometryBuffer_T)) 
			FATAL("Global Geometry buffer overflow" )
		if (indexBuffer.stagingBuffer.size < nbAllocatedIndices * sizeof(IndexBuffer_T)) 
			FATAL("Global Index buffer overflow" )
		if (posBuffer.stagingBuffer.size < nbAllocatedVertices * sizeof(PosBuffer_T)) 
			FATAL("Global Pos buffer overflow" )
		if (normalBuffer.stagingBuffer.size < nbAllocatedVertices * sizeof(NormalBuffer_T)) 
			FATAL("Global Normal buffer overflow" )
		if (colorBuffer.stagingBuffer.size < nbAllocatedVertices * sizeof(ColorBuffer_T)) 
			FATAL("Global Color buffer overflow" )
		
		//TODO dynamically allocate more memory instead of crashing
		
		//
		return geometry->geometryOffset;
	}

	int Geometry::GlobalGeometryBuffers::AddObject(ObjectInstance* obj) {
		std::scoped_lock lock(objectBufferMutex);

		obj->objectOffset = nbAllocatedObjects;
		for (auto [i, l] : objectAllocations) {
			if (!l) {
				obj->objectOffset = i;
				break;
			}
		}
		nbAllocatedObjects = std::max(nbAllocatedObjects, obj->objectOffset + 1);
		objectAllocations[obj->objectOffset] = obj;
		
		// Check for overflow
		if (objectBuffer.stagingBuffer.size < nbAllocatedObjects * sizeof(ObjectBuffer_T)) 
			FATAL("Global Object buffer overflow" )
		
		//TODO dynamically allocate more memory instead of crashing
		
		//
		return obj->objectOffset;
	}
	
	int Geometry::GlobalGeometryBuffers::AddLight(LightSource* lightSource) {
		std::scoped_lock lock(lightBufferMutex);

		lightSource->lightOffset = nbAllocatedLights;
		for (auto [i, l] : lightAllocations) {
			if (!l) {
				lightSource->lightOffset = i;
				break;
			}
		}
		nbAllocatedLights = std::max(nbAllocatedLights, lightSource->lightOffset + 1);
		lightAllocations[lightSource->lightOffset] = lightSource;
		
		// Check for overflow
		if (lightBuffer.stagingBuffer.size < nbAllocatedLights * sizeof(LightBuffer_T)) 
			FATAL("Global Light buffer overflow" )
		
		//TODO dynamically allocate more memory instead of crashing
		
		//
		return lightSource->lightOffset;
	}
	
	void Geometry::GlobalGeometryBuffers::RemoveGeometry(Geometry* geometry) {
		std::scoped_lock lock(geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		geometryAllocations[geometry->geometryOffset] = nullptr;
		indexAllocations[geometry->indexOffset].data = nullptr;
		vertexAllocations[geometry->vertexOffset].data = nullptr;
	}

	void Geometry::GlobalGeometryBuffers::RemoveObject(ObjectInstance* obj) {
		std::scoped_lock lock(objectBufferMutex);
		objectAllocations[obj->objectOffset] = nullptr;
	}

	void Geometry::GlobalGeometryBuffers::RemoveLight(LightSource* lightSource) {
		std::scoped_lock lock(lightBufferMutex);
		lightAllocations[lightSource->lightOffset] = nullptr;
	}
	
	void Geometry::GlobalGeometryBuffers::Allocate(Device* device) {
		objectBuffer.Allocate(device);
		geometryBuffer.Allocate(device);
		indexBuffer.Allocate(device);
		posBuffer.Allocate(device);
		normalBuffer.Allocate(device);
		colorBuffer.Allocate(device);
		lightBuffer.Allocate(device);
	}
	
	void Geometry::GlobalGeometryBuffers::Free(Device* device) {
		objectBuffer.Free(device);
		geometryBuffer.Free(device);
		indexBuffer.Free(device);
		posBuffer.Free(device);
		normalBuffer.Free(device);
		colorBuffer.Free(device);
		lightBuffer.Free(device);
	}
	
	void Geometry::GlobalGeometryBuffers::PushEverything(Device* device, VkCommandBuffer commandBuffer) {
		PushAllGeometries(device, commandBuffer);
		PushObjects(device, commandBuffer);
		PushLights(device, commandBuffer);
	}
	
	void Geometry::GlobalGeometryBuffers::PullEverything(Device* device, VkCommandBuffer commandBuffer) {
		PullAllGeometries(device, commandBuffer);
		PullObjects(device, commandBuffer);
		PullLights(device, commandBuffer);
	}
	
	void Geometry::GlobalGeometryBuffers::PushAllGeometries(Device* device, VkCommandBuffer commandBuffer) {
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Push(device, commandBuffer, nbAllocatedGeometries);
		indexBuffer.Push(device, commandBuffer, nbAllocatedIndices);
		posBuffer.Push(device, commandBuffer, nbAllocatedVertices);
		normalBuffer.Push(device, commandBuffer, nbAllocatedVertices);
		colorBuffer.Push(device, commandBuffer, nbAllocatedVertices);
	}
	
	void Geometry::GlobalGeometryBuffers::PullAllGeometries(Device* device, VkCommandBuffer commandBuffer) {
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Pull(device, commandBuffer, nbAllocatedGeometries);
		indexBuffer.Pull(device, commandBuffer, nbAllocatedIndices);
		posBuffer.Pull(device, commandBuffer, nbAllocatedVertices);
		normalBuffer.Pull(device, commandBuffer, nbAllocatedVertices);
		colorBuffer.Pull(device, commandBuffer, nbAllocatedVertices);
	}
	
	void Geometry::GlobalGeometryBuffers::PushGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
						GeometryBuffersMask geometryBuffersMask, 
						uint32_t vertexCount, uint32_t vertexOffset,
						uint32_t indexCount, uint32_t indexOffset
	) {
		if (vertexCount == 0) vertexCount = geometry->vertexCount;
		else vertexCount = std::min(vertexCount, geometry->vertexCount);
		
		if (indexCount == 0) indexCount = geometry->indexCount;
		else indexCount = std::min(indexCount, geometry->indexCount);
		
		if (geometryBuffersMask & BUFFER_GEOMETRY_INFO) geometryBuffer.Push(device, commandBuffer, 1, geometry->geometryOffset);
		if (geometryBuffersMask & BUFFER_INDEX) indexBuffer.Push(device, commandBuffer, indexCount, geometry->indexOffset + indexOffset);
		if (geometryBuffersMask & BUFFER_POS) posBuffer.Push(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
		if (geometryBuffersMask & BUFFER_NORM) normalBuffer.Push(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
		if (geometryBuffersMask & BUFFER_COLOR) colorBuffer.Push(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
	}
	
	void Geometry::GlobalGeometryBuffers::PullGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
						GeometryBuffersMask geometryBuffersMask, 
						uint32_t vertexCount, uint32_t vertexOffset,
						uint32_t indexCount, uint32_t indexOffset
	) {
		if (vertexCount == 0) vertexCount = geometry->vertexCount;
		else vertexCount = std::min(vertexCount, geometry->vertexCount);
		
		if (indexCount == 0) indexCount = geometry->indexCount;
		else indexCount = std::min(indexCount, geometry->indexCount);
		
		if (geometryBuffersMask & BUFFER_GEOMETRY_INFO) geometryBuffer.Pull(device, commandBuffer, 1, geometry->geometryOffset);
		if (geometryBuffersMask & BUFFER_INDEX) indexBuffer.Pull(device, commandBuffer, indexCount, geometry->indexOffset + indexOffset);
		if (geometryBuffersMask & BUFFER_POS) posBuffer.Pull(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
		if (geometryBuffersMask & BUFFER_NORM) normalBuffer.Pull(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
		if (geometryBuffersMask & BUFFER_COLOR) colorBuffer.Pull(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
	}

	void Geometry::GlobalGeometryBuffers::WriteObject(ObjectInstance* obj) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Writing Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		objData->modelViewMatrix = obj->modelViewMatrix;
		// objData->normalMatrix = obj->normalMatrix; // This does not work... need to do more debugging...
	}
	
	void Geometry::GlobalGeometryBuffers::ReadObject(ObjectInstance* obj) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Reading Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		obj->modelViewMatrix = objData->modelViewMatrix;
		// obj->normalMatrix = objData->normalMatrix; // This does not work... need to do more debugging...
	}
	
	void Geometry::GlobalGeometryBuffers::WriteLight(LightSource* lightSource) {
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Writing Light Sources")
		}
		
		LightBuffer_T* lightData = &((LightBuffer_T*) (globalBuffers.lightBuffer.stagingBuffer.data)) [lightSource->lightOffset];
		
		lightData->position = lightSource->viewSpacePosition;
		lightData->intensity = lightSource->intensity;
		lightData->colorAndType = PackColor(glm::vec4(lightSource->color, 0));
		lightData->colorAndType |= lightSource->type & 0x000000ff;
		lightData->attributes = lightSource->attributes;
		lightData->radius = lightSource->radius;
	}
	
	void Geometry::GlobalGeometryBuffers::ReadLight(LightSource* lightSource) {
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Reading Light Sources")
		}
		
		LightBuffer_T* lightData = &((LightBuffer_T*) (globalBuffers.lightBuffer.stagingBuffer.data)) [lightSource->lightOffset];
		
		lightSource->viewSpacePosition = lightData->position;
		lightSource->intensity = lightData->intensity;
		lightSource->color = glm::vec3(UnpackColor(lightData->colorAndType));
		lightSource->type = lightData->colorAndType & 0x000000ff;
		lightSource->attributes = lightData->attributes;
		lightSource->radius = lightData->radius;
	}

	void Geometry::GlobalGeometryBuffers::PushLights(Device* device, VkCommandBuffer commandBuffer) {
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pushing Light Sources")
		}
		lightBuffer.Push(device, commandBuffer, nbAllocatedLights);
	}

	void Geometry::GlobalGeometryBuffers::PullLights(Device* device, VkCommandBuffer commandBuffer) {
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pulling Light Sources")
		}
		lightBuffer.Pull(device, commandBuffer, nbAllocatedLights);
	}
	
	void Geometry::GlobalGeometryBuffers::PushObjects(Device* device, VkCommandBuffer commandBuffer) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pushing Objects")
		}
		objectBuffer.Push(device, commandBuffer, nbAllocatedObjects);
	}
	
	void Geometry::GlobalGeometryBuffers::PullObjects(Device* device, VkCommandBuffer commandBuffer) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pulling Objects")
		}
		objectBuffer.Pull(device, commandBuffer, nbAllocatedObjects);
	}
	
	void Geometry::GlobalGeometryBuffers::DefragmentMemory() {
		std::scoped_lock lock(geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		//TODO merge free space, and maybe trim the end
	}
	
	#pragma endregion

}
