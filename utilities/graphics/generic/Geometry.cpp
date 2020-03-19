#include <v4d.h>

namespace v4d::graphics {
	
	Geometry::GlobalGeometryBuffers Geometry::globalBuffers {};

	std::unordered_map<std::string, uint32_t> Geometry::rayTracingShaderOffsets {};

	VkGeometryNV Geometry::GetRayTracingGeometry() const {
		VkGeometryNV geometry {};
		geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		
		geometry.flags = 0;//VK_GEOMETRY_OPAQUE_BIT_NV;
			
		if (isProcedural) {
			geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
			
			geometry.geometry.aabbs.numAABBs = vertexCount;
			geometry.geometry.aabbs.stride = sizeof(ProceduralVertexBuffer_T);
			geometry.geometry.aabbs.offset = (VkDeviceSize)(vertexOffset*sizeof(ProceduralVertexBuffer_T));
			
			geometry.geometry.aabbs.aabbData = globalBuffers.vertexBuffer.deviceLocalBuffer.buffer;
			
		} else {
			geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
			
			geometry.geometry.triangles.vertexOffset = (VkDeviceSize)(vertexOffset*sizeof(VertexBuffer_T));
			geometry.geometry.triangles.vertexCount = vertexCount;
			geometry.geometry.triangles.vertexStride = sizeof(VertexBuffer_T);
			geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			geometry.geometry.triangles.indexOffset = (VkDeviceSize)(indexOffset*sizeof(IndexBuffer_T));
			geometry.geometry.triangles.indexCount = indexCount;
			geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			
			geometry.geometry.triangles.vertexData = globalBuffers.vertexBuffer.deviceLocalBuffer.buffer;
			geometry.geometry.triangles.indexData = globalBuffers.indexBuffer.deviceLocalBuffer.buffer;
			
			geometry.geometry.triangles.transformData = transformBuffer? transformBuffer->buffer : VK_NULL_HANDLE;
			geometry.geometry.triangles.transformOffset = transformOffset;
		}
		return geometry;
	}
	
	Geometry::Geometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material, bool isProcedural)
	 : vertexCount(vertexCount), indexCount(indexCount), material(material), isProcedural(isProcedural) {
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
		vertices =			&((VertexBuffer_T*)		(globalBuffers.vertexBuffer.stagingBuffer.data))	[vertexOffset];
		
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
	
	void Geometry::SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color) {
		if (!vertices) MapStagingBuffers();
		
		vertices[i].pos = pos;
		vertices[i].SetColor(color);
		vertices[i].normal = normal;
		vertices[i].SetUV(uv);
	}
	
	void Geometry::SetProceduralVertex(uint32_t i, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1) {
		if (!vertices) MapStagingBuffers();
		
		auto* vert = GetProceduralVertexPtr(i);
		vert->aabbMin = aabbMin;
		vert->aabbMax = aabbMax;
		vert->SetColor(color);
		vert->custom1 = custom1;
	}
	
	void Geometry::SetIndex(uint32_t i, IndexBuffer_T vertexIndex) {
		if (!indices) MapStagingBuffers();
		
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
		
		if (pos) *pos = vertices[i].pos;
		if (normal) *normal = vertices[i].normal;
		if (uv) *uv = vertices[i].GetUV();
		if (color) *color = vertices[i].GetColor();
	}

	Geometry::VertexBuffer_T* Geometry::GetVertexPtr(uint32_t i) {
		if (!vertices) MapStagingBuffers();
		return &vertices[i];
	}
	
	void Geometry::GetProceduralVertex(uint32_t i, glm::vec3* aabbMin, glm::vec3* aabbMax, glm::vec4* color, float* custom1) {
		if (!vertices) MapStagingBuffers();
		
		auto* vert = GetProceduralVertexPtr(i);
		
		if (aabbMin) *aabbMin = vert->aabbMin;
		if (aabbMax) *aabbMax = vert->aabbMax;
		if (color) *color = vert->GetColor();
		if (custom1) *custom1 = vert->custom1;
	}
	
	Geometry::ProceduralVertexBuffer_T* Geometry::GetProceduralVertexPtr(uint32_t i) {
		if (!vertices) MapStagingBuffers();
		return &((ProceduralVertexBuffer_T*)vertices)[i];
	}
	
	void Geometry::GetIndex(uint32_t i, IndexBuffer_T* vertexIndex) {
		if (!indices) MapStagingBuffers();
		
		if (vertexIndex) *vertexIndex = indices[i];
	}

	Geometry::IndexBuffer_T Geometry::GetIndex(uint32_t i) {
		if (!indices) MapStagingBuffers();
		
		return indices[i];
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
		if (geometry->indexCount > 0) {
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
		}
		
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
		if (vertexBuffer.stagingBuffer.size < nbAllocatedVertices * sizeof(VertexBuffer_T)) 
			FATAL("Global Pos buffer overflow" )
		
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
		vertexBuffer.Allocate(device);
		lightBuffer.Allocate(device);
	}
	
	void Geometry::GlobalGeometryBuffers::Free(Device* device) {
		objectBuffer.Free(device);
		geometryBuffer.Free(device);
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
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Push(device, commandBuffer, nbAllocatedGeometries);
		indexBuffer.Push(device, commandBuffer, nbAllocatedIndices);
		vertexBuffer.Push(device, commandBuffer, nbAllocatedVertices);
	}
	
	void Geometry::GlobalGeometryBuffers::PullAllGeometries(Device* device, VkCommandBuffer commandBuffer) {
		if (nbAllocatedGeometries == 0) return;
		geometryBuffer.Pull(device, commandBuffer, nbAllocatedGeometries);
		indexBuffer.Pull(device, commandBuffer, nbAllocatedIndices);
		vertexBuffer.Pull(device, commandBuffer, nbAllocatedVertices);
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
		if (geometryBuffersMask & BUFFER_VERTEX) vertexBuffer.Push(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
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
		if (geometryBuffersMask & BUFFER_VERTEX) vertexBuffer.Pull(device, commandBuffer, vertexCount, geometry->vertexOffset + vertexOffset);
	}

	void Geometry::GlobalGeometryBuffers::WriteObject(ObjectInstance* obj) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Writing Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		objData->modelViewMatrix = obj->modelViewMatrix;
		objData->normalMatrix = obj->normalMatrix;
		objData->custom4 = obj->custom4;
		objData->custom4 = obj->custom4;
	}
	
	void Geometry::GlobalGeometryBuffers::ReadObject(ObjectInstance* obj) {
		if (!globalBuffers.objectBuffer.stagingBuffer.data) {
			FATAL("global buffers must be allocated before Reading Objects")
		}
		
		ObjectBuffer_T* objData = &((ObjectBuffer_T*) (globalBuffers.objectBuffer.stagingBuffer.data)) [obj->objectOffset];
		
		obj->modelViewMatrix = objData->modelViewMatrix;
		obj->normalMatrix = objData->normalMatrix;
		obj->normalMatrix = objData->normalMatrix;
		obj->custom3 = objData->custom3;
		obj->custom4 = objData->custom4;
	}
	
	void Geometry::GlobalGeometryBuffers::WriteLight(LightSource* lightSource) {
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
	
	
	#pragma region Pack Helpers
	
	/////////////////
	// NOT WORKING
			// static NormalBuffer_T PackNormal(glm::vec3 normal) {
			// 	// // vec2
			// 	// float f = glm::sqrt(8.0f * normal.z + 8.0f);
			// 	// return glm::vec2(normal) / f + 0.5f;
				
			// 	// vec4
			// 	return glm::vec4(normal, 0);
			// }

			// static glm::vec3 UnpackNormal(NormalBuffer_T norm) {
			// 	// glm::vec2 fenc = norm * 4.0f - 2.0f;
			// 	// float f = glm::dot(fenc, fenc);
			// 	// float g = glm::sqrt(1.0f - f / 4.0f);
			// 	// return glm::vec3(fenc * g, 1.0f - f / 2.0f);
			// 	return norm;
			// }
	/////////////////

	glm::f32 PackColorAsFloat(glm::vec4 color) {
		color *= 255.0f;
		glm::uvec4 pack {
			glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
		};
		return glm::uintBitsToFloat((pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a);
	}

	glm::u32 PackColorAsUint(glm::vec4 color) {
		color *= 255.0f;
		glm::uvec4 pack {
			glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
			glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
		};
		return (pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a;
	}

	glm::vec4 UnpackColorFromFloat(glm::f32 color) {
		glm::u32 packed = glm::floatBitsToUint(color);
		return glm::vec4(
			(packed & 0xff000000) >> 24,
			(packed & 0x00ff0000) >> 16,
			(packed & 0x0000ff00) >> 8,
			(packed & 0x000000ff) >> 0
		) / 255.0f;
	}
	
	glm::vec4 UnpackColorFromUint(glm::u32 color) {
		return glm::vec4(
			(color & 0xff000000) >> 24,
			(color & 0x00ff0000) >> 16,
			(color & 0x0000ff00) >> 8,
			(color & 0x000000ff) >> 0
		) / 255.0f;
	}
	
	glm::f32 PackUVasFloat(glm::vec2 uv) {
		uv *= 65535.0f;
		glm::uvec2 pack {
			glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
			glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
		};
		return glm::uintBitsToFloat((pack.s << 16) | pack.t);
	}

	glm::u32 PackUVasUint(glm::vec2 uv) {
		uv *= 65535.0f;
		glm::uvec2 pack {
			glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
			glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
		};
		return (pack.s << 16) | pack.t;
	}

	glm::vec2 UnpackUVfromFloat(glm::f32 uv) {
		glm::u32 packed = glm::floatBitsToUint(uv);
		return glm::vec2(
			(packed & 0xffff0000) >> 16,
			(packed & 0x0000ffff) >> 0
		) / 65535.0f;
	}

	glm::vec2 UnpackUVfromUint(glm::u32 uv) {
		return glm::vec2(
			(uv & 0xffff0000) >> 16,
			(uv & 0x0000ffff) >> 0
		) / 65535.0f;
	}

	#pragma endregion


}
