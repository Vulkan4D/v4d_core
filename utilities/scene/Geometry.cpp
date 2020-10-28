#include <v4d.h>

namespace v4d::scene {
	
	Geometry::GlobalGeometryBuffers Geometry::globalBuffers {};

	std::unordered_map<std::string, GeometryRenderType> Geometry::geometryRenderTypes {};

	Geometry::Geometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material, bool isProcedural)
	 : vertexCount(vertexCount), indexCount(indexCount), material(material), isProcedural(isProcedural) {
		globalBuffers.AddGeometry(this);
	}
	
	Geometry::Geometry(std::shared_ptr<Geometry> duplicateFrom)
	 : vertexCount(0), indexCount(0), material(duplicateFrom->material), isProcedural(duplicateFrom->isProcedural), duplicateFrom(duplicateFrom) {
		globalBuffers.AddGeometry(this);
		vertexCount = duplicateFrom->vertexCount;
		indexCount = duplicateFrom->indexCount;
		vertexOffset = duplicateFrom->vertexOffset;
		indexOffset = duplicateFrom->indexOffset;
	}
	
	Geometry::~Geometry() {
		if (duplicateFrom) {
			vertexCount = 0;
			indexCount = 0;
		}
		globalBuffers.RemoveGeometry(this);
		
		// if (blas.use_count() == 1) LOG_VERBOSE("Destroyed Geometry with Acceleration Structure handle " << std::hex << blas->handle)
	}
	
	void Geometry::Shrink(uint32_t newVertexCount, uint32_t newIndexCount) {
		globalBuffers.ShrinkGeometryVertices(this, newVertexCount);
		globalBuffers.ShrinkGeometryIndices(this, newIndexCount);
	}
	
	void Geometry::MapStagingBuffers() {
		
		//TODO only allocate and map staging buffers if it is needed, and remove the fatal error
		if (!globalBuffers.geometryBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before mapping staging buffers")
		}
		
		geometryInfo =		&((GeometryBuffer_T*)	(globalBuffers.geometryBuffer.stagingBuffer.data))	[geometryOffset];
		
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			indices =	&((IndexBuffer_T*)	(globalBuffers.indexBuffer.stagingBuffer.data))		[indexOffset];
			vertices =	&((VertexBuffer_T*)	(globalBuffers.vertexBuffer.stagingBuffer.data))	[vertexOffset];
		#else
			indices =	&((IndexBuffer_T*)	(globalBuffers.indexBuffer.data))	[indexOffset];
			vertices =	&((VertexBuffer_T*)	(globalBuffers.vertexBuffer.data))	[vertexOffset];
		#endif
		
		// LOG("Global Geometry Staging Buffers mapped")
	}
	
	void Geometry::UnmapStagingBuffers() {
		geometryInfo = nullptr;
		indices = nullptr;
		vertices = nullptr;
		
		geometryInfoInitialized = false;
	}
	
	void Geometry::SetGeometryInfo(uint32_t objectIndex, uint32_t material) {
		if (!geometryInfo) MapStagingBuffers();
		
		geometryInfo->indexOffset = indexOffset;
		geometryInfo->vertexOffset = vertexOffset;
		geometryInfo->objectIndex = objectIndex;
		geometryInfo->material = material;
		
		geometryInfoInitialized = true;
	}
	
	void Geometry::SetGeometryTransform(glm::dmat4 modelTransform, const glm::dmat4& viewMatrix) {
		if (!geometryInfo) MapStagingBuffers();
		
		geometryInfo->modelTransform = modelTransform;
		geometryInfo->modelViewTransform = viewMatrix * modelTransform;
		geometryInfo->normalViewTransform = glm::transpose(glm::inverse(glm::mat3(geometryInfo->modelViewTransform)));
		geometryInfo->custom3f = custom3f;
		geometryInfo->custom4x4f = custom4x4f;
	}
	
	void Geometry::SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		
		vertices[i].pos = pos;
		vertices[i].SetColor(color);
		vertices[i].normal = normal;
		vertices[i].SetUV(uv);
		
		boundingDistance = glm::max(boundingDistance, glm::length(pos));
		boundingBoxSize = glm::max(glm::abs(pos), boundingBoxSize);
		
		isDirty = true;
	}
	
	void Geometry::SetProceduralVertex(uint32_t i, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		
		auto* vert = GetProceduralVertexPtr(i);
		vert->aabbMin = aabbMin;
		vert->aabbMax = aabbMax;
		vert->SetColor(color);
		vert->custom1 = custom1;
		
		boundingDistance = glm::max(boundingDistance, glm::max(glm::length(aabbMin), glm::length(aabbMax)));
		boundingBoxSize = glm::max(boundingBoxSize, (aabbMax - aabbMin) / 2.0f);
		
		isDirty = true;
	}
	
	void Geometry::SetIndex(uint32_t i, IndexBuffer_T vertexIndex) {
		if (!indices) MapStagingBuffers();
		DEBUG_ASSERT(i < indexCount)
		
		indices[i] = vertexIndex;
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
	
	void Geometry::GetGeometryInfo(uint32_t* objectIndex, uint32_t* material) {
		if (!geometryInfo) MapStagingBuffers();
		
		indexOffset = geometryInfo->indexOffset;
		vertexOffset = geometryInfo->vertexOffset;
		if (objectIndex) *objectIndex = geometryInfo->objectIndex;
		if (material) *material = geometryInfo->material;
	}
	
	void Geometry::GetVertex(uint32_t i, glm::vec3* pos, glm::vec3* normal, glm::vec2* uv, glm::vec4* color) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		
		if (pos) *pos = vertices[i].pos;
		if (normal) *normal = vertices[i].normal;
		if (uv) *uv = vertices[i].GetUV();
		if (color) *color = vertices[i].GetColor();
	}

	Geometry::VertexBuffer_T* Geometry::GetVertexPtr(uint32_t i) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		return &vertices[i];
	}
	
	void Geometry::GetProceduralVertex(uint32_t i, glm::vec3* aabbMin, glm::vec3* aabbMax, glm::vec4* color, float* custom1) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		
		auto* vert = GetProceduralVertexPtr(i);
		
		if (aabbMin) *aabbMin = vert->aabbMin;
		if (aabbMax) *aabbMax = vert->aabbMax;
		if (color) *color = vert->GetColor();
		if (custom1) *custom1 = vert->custom1;
	}
	
	Geometry::ProceduralVertexBuffer_T* Geometry::GetProceduralVertexPtr(uint32_t i) {
		if (!vertices) MapStagingBuffers();
		DEBUG_ASSERT(i < vertexCount)
		return &((ProceduralVertexBuffer_T*)vertices)[i];
	}
	
	void Geometry::GetIndex(uint32_t i, IndexBuffer_T* vertexIndex) {
		if (!indices) MapStagingBuffers();
		DEBUG_ASSERT(i < indexCount)
		if (vertexIndex) *vertexIndex = indices[i];
	}

	Geometry::IndexBuffer_T Geometry::GetIndex(uint32_t i) {
		if (!indices) MapStagingBuffers();
		DEBUG_ASSERT(i < indexCount)
		return indices[i];
	}

	Geometry::IndexBuffer_T* Geometry::GetIndexPtr(uint32_t i) {
		if (!indices) MapStagingBuffers();
		DEBUG_ASSERT(i < indexCount)
		return &indices[i];
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
	

	void Geometry::AutoPush(Device* device, VkCommandBuffer commandBuffer, bool forcePushTransform) {
		if (isDirty) {
			if (duplicateFrom) {
				duplicateFrom->AutoPush(device, commandBuffer);
				Push(device, commandBuffer, GlobalGeometryBuffers::BUFFER_GEOMETRY_INFO);
			} else {
				Push(device, commandBuffer);
			}
			isDirty = false;
		} else if (forcePushTransform) {
			Push(device, commandBuffer, GlobalGeometryBuffers::BUFFER_GEOMETRY_INFO);
		}
	}
	
	void Geometry::Push(Device* device, VkCommandBuffer commandBuffer, 
			GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				, uint32_t vertexCount, uint32_t vertexOffset
				, uint32_t indexCount, uint32_t indexOffset
			#endif
		) {
		globalBuffers.PushGeometry(device, commandBuffer, this, geometryBuffersMask
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				, vertexCount, vertexOffset, indexCount, indexOffset
			#endif
		);
	}

	void Geometry::Pull(Device* device, VkCommandBuffer commandBuffer, 
			GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				, uint32_t vertexCount, uint32_t vertexOffset
				, uint32_t indexCount, uint32_t indexOffset
			#endif
		) {
		globalBuffers.PullGeometry(device, commandBuffer, this, geometryBuffersMask
			#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
				, vertexCount, vertexOffset, indexCount, indexOffset
			#endif
		);
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
		if (geometry->indexCount > 0) {
			geometry->indexOffset = nbAllocatedIndices;
			for (auto [i, alloc] : indexAllocations) {
				if (!alloc.data && alloc.n >= geometry->indexCount) {
					geometry->indexOffset = i;
					if (alloc.n > geometry->indexCount) {
						indexAllocations[geometry->indexOffset+geometry->indexCount] = {alloc.n - geometry->indexCount, nullptr};
					}
					break;
				}
			}
			nbAllocatedIndices = std::max(nbAllocatedIndices, geometry->indexOffset + geometry->indexCount);
			indexAllocations[geometry->indexOffset] = {geometry->indexCount, geometry};
		}
		
		// Vertex
		if (geometry->vertexCount > 0) {
			geometry->vertexOffset = nbAllocatedVertices;
			for (auto [i, alloc] : vertexAllocations) {
				if (!alloc.data && alloc.n >= geometry->vertexCount) {
					geometry->vertexOffset = i;
					if (alloc.n > geometry->vertexCount) {
						vertexAllocations[geometry->vertexOffset+geometry->vertexCount] = {alloc.n - geometry->vertexCount, nullptr};
					}
					break;
				}
			}
			nbAllocatedVertices = std::max(nbAllocatedVertices, geometry->vertexOffset + geometry->vertexCount);
			vertexAllocations[geometry->vertexOffset] = {geometry->vertexCount, geometry};
		}
		
		// Check for overflow
		if (geometryBuffer.stagingBuffer.size < nbAllocatedGeometries * sizeof(GeometryBuffer_T)) 
			FATAL("Global Geometry buffer overflow" )
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			if (indexBuffer.stagingBuffer.size < nbAllocatedIndices * sizeof(IndexBuffer_T)) 
				FATAL("Global Index buffer overflow" )
			if (vertexBuffer.stagingBuffer.size < nbAllocatedVertices * sizeof(VertexBuffer_T)) 
				FATAL("Global Vertex buffer overflow" )
		#else
			if (indexBuffer.size < nbAllocatedIndices * sizeof(IndexBuffer_T)) 
				FATAL("Global Index buffer overflow" )
			if (vertexBuffer.size < nbAllocatedVertices * sizeof(VertexBuffer_T)) 
				FATAL("Global Vertex buffer overflow" )
		#endif
		
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
	
	void Geometry::GlobalGeometryBuffers::RemoveObject(ObjectInstance* obj) {
		std::scoped_lock lock(objectBufferMutex);
		objectAllocations[obj->objectOffset] = nullptr;
	}

	void Geometry::GlobalGeometryBuffers::RemoveLight(LightSource* lightSource) {
		std::scoped_lock lock(lightBufferMutex);
		lightAllocations[lightSource->lightOffset] = nullptr;
	}
	
	void Geometry::GlobalGeometryBuffers::RemoveGeometry(Geometry* geometry) {
		// Vertex buffer
		if (geometry->vertexCount > 0) {
			std::lock_guard lock(vertexBufferMutex);
			uint32_t offset = geometry->vertexOffset;
			vertexAllocations[offset].data = nullptr;
			uint32_t count = vertexAllocations[offset].n;
			// Merge next allocation if empty
			try {
				uint32_t nextOffset = offset + count;
				auto& next = vertexAllocations.at(nextOffset);
				if (!next.data && next.n > 0) {
					count = vertexAllocations[offset].n += next.n;
					vertexAllocations.erase(nextOffset);
				}
			} catch(...){} // not an error... just ignore it
			// Shrink total allocation if we were at the tail of the global buffer
			if (nbAllocatedVertices == count + offset) {
				nbAllocatedVertices = offset;
				vertexAllocations.erase(offset);
			}
		}
		// Index buffer
		if (geometry->indexCount > 0) {
			std::lock_guard lock(indexBufferMutex);
			uint32_t offset = geometry->indexOffset;
			indexAllocations[offset].data = nullptr;
			uint32_t count = indexAllocations[offset].n;
			// Merge next allocation if empty
			try {
				uint32_t nextOffset = offset + count;
				auto& next = indexAllocations.at(nextOffset);
				if (!next.data && next.n > 0) {
					count = indexAllocations[offset].n += next.n;
					indexAllocations.erase(nextOffset);
				}
			} catch(...){} // not an error... just ignore it
			// Shrink total allocation if we were at the tail of the global buffer
			if (nbAllocatedIndices == count + offset) {
				nbAllocatedIndices = offset;
				indexAllocations.erase(offset);
			}
		}
		// Geometry buffer
		std::lock_guard lock(geometryBufferMutex);
		geometryAllocations[geometry->geometryOffset] = nullptr;
	}
	
	void Geometry::GlobalGeometryBuffers::ShrinkGeometryVertices(Geometry* geometry, uint32_t newVertexCount) {
		assert(newVertexCount <= geometry->vertexCount);
		assert(newVertexCount > 0);
		if (newVertexCount < geometry->vertexCount) {
			std::lock_guard lock(vertexBufferMutex);
			uint32_t freeCount = geometry->vertexCount - newVertexCount;
			uint32_t freeOffset = geometry->vertexOffset + newVertexCount;
			// Merge next allocation if empty
			try {
				uint32_t nextOffset = freeOffset + freeCount;
				auto& next = vertexAllocations.at(nextOffset);
				if (!next.data && next.n > 0) {
					freeCount += next.n;
					vertexAllocations.erase(nextOffset);
				}
			} catch(...){} // not an error... just ignore it
			// Assign free space
			vertexAllocations[freeOffset].data = nullptr;
			vertexAllocations[freeOffset].n = freeCount;
			vertexAllocations[geometry->vertexOffset].n = newVertexCount;
			// Shrink total allocation if we were at the tail of the global buffer
			if (nbAllocatedVertices == freeCount) {
				nbAllocatedVertices = geometry->vertexOffset + newVertexCount;
				vertexAllocations.erase(freeOffset);
			}
			// Assign new count
			geometry->vertexCount = newVertexCount;
		}
	}
	
	void Geometry::GlobalGeometryBuffers::ShrinkGeometryIndices(Geometry* geometry, uint32_t newIndexCount) {
		assert(newIndexCount <= geometry->indexCount);
		assert(newIndexCount > 0);
		if (newIndexCount < geometry->indexCount) {
			std::lock_guard lock(indexBufferMutex);
			uint32_t freeCount = geometry->indexCount - newIndexCount;
			uint32_t freeOffset = geometry->indexOffset + newIndexCount;
			// Merge next allocation if empty
			try {
				uint32_t nextOffset = freeOffset + freeCount;
				auto& next = indexAllocations.at(nextOffset);
				if (!next.data && next.n > 0) {
					freeCount += next.n;
					indexAllocations.erase(nextOffset);
				}
			} catch(...) {} // not an error... just ignore it
			// Assign free space
			indexAllocations[freeOffset].data = nullptr;
			indexAllocations[freeOffset].n = freeCount;
			indexAllocations[geometry->indexOffset].n = newIndexCount;
			// Shrink total allocation if we were at the tail of the global buffer
			if (nbAllocatedIndices == freeCount) {
				nbAllocatedIndices = geometry->indexOffset + newIndexCount;
				indexAllocations.erase(freeOffset);
			}
			// Assign new count
			geometry->indexCount = newIndexCount;
		}
	}

	void Geometry::GlobalGeometryBuffers::Allocate(Device* device, const std::vector<uint32_t>& queueFamilies) {
		std::scoped_lock lock(objectBufferMutex, lightBufferMutex, geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		objectBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queueFamilies);
		geometryBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queueFamilies);
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			indexBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queueFamilies);
			vertexBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queueFamilies);
		#else
			indexBuffer.Allocate(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false, queueFamilies);
			indexBuffer.MapMemory(device);
			vertexBuffer.Allocate(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false, queueFamilies);
			vertexBuffer.MapMemory(device);
		#endif
		lightBuffer.Allocate(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, queueFamilies);
	}
	
	void Geometry::GlobalGeometryBuffers::Free(Device* device) {
		std::scoped_lock lock(objectBufferMutex, lightBufferMutex, geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		objectBuffer.Free(device);
		geometryBuffer.Free(device);
		#ifndef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			indexBuffer.UnmapMemory(device);
			vertexBuffer.UnmapMemory(device);
		#endif
		indexBuffer.Free(device);
		vertexBuffer.Free(device);
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
		std::scoped_lock lock(geometryBufferMutex, indexBufferMutex, vertexBufferMutex);
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Push(device, commandBuffer, nbAllocatedGeometries);
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			indexBuffer.Push(device, commandBuffer, nbAllocatedIndices);
			vertexBuffer.Push(device, commandBuffer, nbAllocatedVertices);
		#endif
	}
	
	void Geometry::GlobalGeometryBuffers::PullAllGeometries(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(geometryBufferMutex, indexBufferMutex, vertexBufferMutex);
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Pull(device, commandBuffer, nbAllocatedGeometries);
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			indexBuffer.Pull(device, commandBuffer, nbAllocatedIndices);
			vertexBuffer.Pull(device, commandBuffer, nbAllocatedVertices);
		#endif
	}
	
	void Geometry::GlobalGeometryBuffers::PushGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
						GeometryBuffersMask geometryBuffersMask
						#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
							, uint32_t vertexCount, uint32_t vertexOffset
							, uint32_t indexCount, uint32_t indexOffset
						#endif
	) {
		std::scoped_lock lock(geometryBufferMutex, indexBufferMutex, vertexBufferMutex);
		if (geometryBuffersMask & BUFFER_GEOMETRY_INFO) geometryBuffer.Push(device, commandBuffer, 1, geometry->geometryOffset);
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			if (geometryBuffersMask & BUFFER_INDEX) {
				if (indexCount == 0) indexCount = geometry->indexCount;
				else indexCount = std::min(indexCount, geometry->indexCount);
				indexBuffer.Push(device, commandBuffer, indexCount, geometry->indexOffset + indexOffset);
			}
			if (geometryBuffersMask & BUFFER_VERTEX) {
				if (vertexCount == 0) vertexCount = geometry->vertexCount;
				else vertexCount = std::min(vertexCount, geometry->vertexCount);
				vertexBuffer.Push(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
			}
		#endif
	}
	
	void Geometry::GlobalGeometryBuffers::PullGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
						GeometryBuffersMask geometryBuffersMask
						#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
							, uint32_t vertexCount, uint32_t vertexOffset
							, uint32_t indexCount, uint32_t indexOffset
						#endif
	) {
		std::scoped_lock lock(geometryBufferMutex, indexBufferMutex, vertexBufferMutex);
		if (geometryBuffersMask & BUFFER_GEOMETRY_INFO) geometryBuffer.Pull(device, commandBuffer, 1, geometry->geometryOffset);
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			if (geometryBuffersMask & BUFFER_INDEX) {
				if (indexCount == 0) indexCount = geometry->indexCount;
				else indexCount = std::min(indexCount, geometry->indexCount);
				indexBuffer.Pull(device, commandBuffer, indexCount, geometry->indexOffset + indexOffset);
			}
			if (geometryBuffersMask & BUFFER_VERTEX) {
				if (vertexCount == 0) vertexCount = geometry->vertexCount;
				else vertexCount = std::min(vertexCount, geometry->vertexCount);
				vertexBuffer.Pull(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
			}
		#endif
	}

	void Geometry::GlobalGeometryBuffers::WriteObject(ObjectInstance* obj) {
		std::scoped_lock lock(objectBufferMutex);
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Writing Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		objData->modelTransform = obj->transform;
	}
	
	void Geometry::GlobalGeometryBuffers::ReadObject(ObjectInstance* obj) {
		std::scoped_lock lock(objectBufferMutex);
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Reading Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		obj->transform = objData->modelTransform;
	}
	
	void Geometry::GlobalGeometryBuffers::WriteLight(LightSource* lightSource) {
		std::scoped_lock lock(lightBufferMutex);
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Writing Light Sources")
		}
		
		LightBuffer_T* lightData = &((LightBuffer_T*) (globalBuffers.lightBuffer.stagingBuffer.data)) [lightSource->lightOffset];
		
		lightData->position = lightSource->viewSpacePosition;
		lightData->intensity = lightSource->intensity;
		lightData->SetColorAndType(lightSource->color, lightSource->type);
		lightData->attributes = lightSource->attributes;
		lightData->radius = lightSource->radius;
		lightData->custom1 = lightSource->custom1;
	}
	
	void Geometry::GlobalGeometryBuffers::ReadLight(LightSource* lightSource) {
		std::scoped_lock lock(lightBufferMutex);
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Reading Light Sources")
		}
		
		LightBuffer_T* lightData = &((LightBuffer_T*) (globalBuffers.lightBuffer.stagingBuffer.data)) [lightSource->lightOffset];
		
		lightSource->viewSpacePosition = lightData->position;
		lightSource->intensity = lightData->intensity;
		lightData->GetColorAndType(&lightSource->color, &lightSource->type);
		lightSource->attributes = lightData->attributes;
		lightSource->radius = lightData->radius;
		lightSource->custom1 = lightData->custom1;
	}

	void Geometry::GlobalGeometryBuffers::PushLights(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(lightBufferMutex);
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pushing Light Sources")
		}
		lightBuffer.Push(device, commandBuffer, nbAllocatedLights);
	}

	void Geometry::GlobalGeometryBuffers::PullLights(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(lightBufferMutex);
		if (!globalBuffers.lightBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pulling Light Sources")
		}
		lightBuffer.Pull(device, commandBuffer, nbAllocatedLights);
	}
	
	void Geometry::GlobalGeometryBuffers::PushObjects(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(objectBufferMutex);
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pushing Objects")
		}
		objectBuffer.Push(device, commandBuffer, nbAllocatedObjects);
	}
	
	void Geometry::GlobalGeometryBuffers::PullObjects(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(objectBufferMutex);
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pulling Objects")
		}
		objectBuffer.Pull(device, commandBuffer, nbAllocatedObjects);
	}
	
	void Geometry::GlobalGeometryBuffers::PushGeometriesInfo(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(geometryBufferMutex);
		if (!globalBuffers.geometryBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pushing Geometries")
		}
		geometryBuffer.Push(device, commandBuffer, nbAllocatedGeometries);
	}
	
	void Geometry::GlobalGeometryBuffers::PullGeometriesInfo(Device* device, VkCommandBuffer commandBuffer) {
		std::scoped_lock lock(geometryBufferMutex);
		if (!globalBuffers.geometryBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Pulling Geometries")
		}
		geometryBuffer.Pull(device, commandBuffer, nbAllocatedGeometries);
	}
	
	void Geometry::GlobalGeometryBuffers::DefragmentMemory() {
		std::scoped_lock lock(objectBufferMutex, lightBufferMutex, geometryBufferMutex, vertexBufferMutex, indexBufferMutex);
		
		{// Defragment Objects
			for (uint32_t i = 0; i < nbAllocatedObjects; ++i) {
				if (!objectAllocations[i]) {
					for (uint32_t j = i+1; j < nbAllocatedObjects; ++j) {
						if (objectAllocations[j]) {
							objectAllocations[i] = objectAllocations[j];
							objectAllocations[j] = nullptr;
							objectAllocations[i]->objectOffset = i;
							objectAllocations[i]->SetGeometriesDirty();
							break;
						}
					}
					if (!objectAllocations[i]) {
						if (i == 0) {
							objectAllocations.clear();
							nbAllocatedObjects = 0;
						}
						break;
					}
				}
			}
			// Trim Objects
			while (nbAllocatedObjects > 0 && !objectAllocations[nbAllocatedObjects - 1]) {
				objectAllocations.erase(nbAllocatedObjects - 1);
				nbAllocatedObjects--;
			}
		}
		
		{// Defragment Lights
			for (uint32_t i = 0; i < nbAllocatedLights; ++i) {
				if (!lightAllocations[i]) {
					for (uint32_t j = i+1; j < nbAllocatedLights; ++j) {
						if (lightAllocations[j]) {
							lightAllocations[i] = lightAllocations[j];
							lightAllocations[j] = nullptr;
							lightAllocations[i]->lightOffset = i;
							break;
						}
					}
					if (!lightAllocations[i]) {
						if (i == 0) {
							lightAllocations.clear();
							nbAllocatedLights = 0;
						}
						break;
					}
				}
			}
			// Trim Lights
			while (nbAllocatedLights > 0 && !lightAllocations[nbAllocatedLights - 1]) {
				lightAllocations.erase(nbAllocatedLights - 1);
				nbAllocatedLights--;
			}
		}
		
		{// Defragment Geometries
			for (uint32_t i = 0; i < nbAllocatedGeometries; ++i) {
				if (!geometryAllocations[i]) {
					for (uint32_t j = i+1; j < nbAllocatedGeometries; ++j) {
						if (geometryAllocations[j]) {
							geometryAllocations[i] = geometryAllocations[j];
							geometryAllocations[j] = nullptr;
							geometryAllocations[i]->geometryOffset = i;
							geometryAllocations[i]->geometryInfoInitialized = false;
							break;
						}
					}
					if (!geometryAllocations[i]) {
						if (i == 0) {
							geometryAllocations.clear();
							nbAllocatedGeometries = 0;
						}
						break;
					}
				}
			}
			// Trim Geometries
			while (nbAllocatedGeometries > 0 && !geometryAllocations[nbAllocatedGeometries - 1]) {
				geometryAllocations.erase(nbAllocatedGeometries - 1);
				nbAllocatedGeometries--;
			}
		}
		
		{// Defragment Indices
			for (uint32_t offset = 0; offset < nbAllocatedIndices; ) {
				try {
					indexAllocations.at(offset);
				} catch(...) {
					LOG_ERROR("GlobalIndexBuffer fragmentation error: overflow at offset " << offset)
					break;
				}
				if (!indexAllocations[offset].data) {
					for (uint32_t nextOffset = offset + indexAllocations[offset].n; nextOffset < nbAllocatedIndices; ) {
						try {
							indexAllocations.at(nextOffset);
						} catch(...) {
							LOG_ERROR("GlobalIndexBuffer fragmentation error: overflow at nextOffset " << nextOffset)
						}
						if (indexAllocations[nextOffset].data) {
							// next allocation is not empty, swap them
							const auto prevAlloc = indexAllocations[offset];
							const auto alloc = indexAllocations[nextOffset];
							indexAllocations[offset] = alloc;
							if (nextOffset != offset+alloc.n) {
								indexAllocations.erase(nextOffset);
								nextOffset = offset+alloc.n;
							}
							indexAllocations[nextOffset] = prevAlloc;
							alloc.data->indexOffset = offset;
							alloc.data->geometryInfoInitialized = false;
							break;
						} else {
							// next allocation is empty, merge them
							const int nextN = indexAllocations[nextOffset].n;
							if (nextN == 0) {
								LOG_ERROR("GlobalIndexBuffer fragmentation error: size zero at offset " << nextOffset)
								break;
							}
							indexAllocations[offset].n += nextN;
							indexAllocations.erase(nextOffset);
							nextOffset += nextN;
						}
					}
				}
				if (indexAllocations[offset].n == 0) break;
				offset += indexAllocations[offset].n;
			}
			// Trim Indices
			if (indexAllocations.size() > 0) {
				auto last = indexAllocations.rbegin();
				if (!last->second.data) {
					nbAllocatedIndices -= last->second.n;
					indexAllocations.erase(last->first);
				}
			}
		}
		
		{// Defragment Vertices
			for (uint32_t offset = 0; offset < nbAllocatedVertices; ) {
				try {
					vertexAllocations.at(offset);
				} catch(...) {
					LOG_ERROR("GlobalVertexBuffer fragmentation error: overflow at offset " << offset)
					break;
				}
				if (!vertexAllocations[offset].data) {
					for (uint32_t nextOffset = offset + vertexAllocations[offset].n; nextOffset < nbAllocatedVertices; ) {
						try {
							vertexAllocations.at(nextOffset);
						} catch(...) {
							LOG_ERROR("GlobalVertexBuffer fragmentation error: overflow at nextOffset " << nextOffset)
						}
						if (vertexAllocations[nextOffset].data) {
							// next allocation is not empty, swap them
							const auto prevAlloc = vertexAllocations[offset];
							const auto alloc = vertexAllocations[nextOffset];
							vertexAllocations[offset] = alloc;
							if (nextOffset != offset+alloc.n) {
								vertexAllocations.erase(nextOffset);
								nextOffset = offset+alloc.n;
							}
							vertexAllocations[nextOffset] = prevAlloc;
							alloc.data->vertexOffset = offset;
							alloc.data->geometryInfoInitialized = false;
							break;
						} else {
							// next allocation is empty, merge them
							const int nextN = vertexAllocations[nextOffset].n;
							if (nextN == 0) {
								LOG_ERROR("GlobalVertexBuffer fragmentation error: size zero at offset " << nextOffset)
								break;
							}
							vertexAllocations[offset].n += nextN;
							vertexAllocations.erase(nextOffset);
							nextOffset += nextN;
						}
					}
				}
				if (vertexAllocations[offset].n == 0) break;
				offset += vertexAllocations[offset].n;
			}
			// Trim Vertices
			if (vertexAllocations.size() > 0) {
				auto last = vertexAllocations.rbegin();
				if (!last->second.data) {
					nbAllocatedVertices -= last->second.n;
					vertexAllocations.erase(last->first);
				}
			}
		}
		
	}
	
	#pragma endregion
	
}
