#include <v4d.h>

namespace v4d::graphics {

	V4D_ENTITY_DEFINE_CLASS(RenderableGeometryEntity)
	
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::GeometryInfo>, meshGeometries)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index16>, meshIndices16)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index32>, meshIndices32)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ProceduralVertexAABB>, proceduralVertexAABB)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexPosition>, meshVertexPosition)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexNormal>, meshVertexNormal)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<uint8_t>>, meshVertexColorU8)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<uint16_t>>, meshVertexColorU16)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<glm::f32>>, meshVertexColorF32)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexUV>, meshVertexUV)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<glm::f32>, meshCustomData)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, RenderableGeometryEntity::LightSource, lightSource)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, v4d::scene::PhysicsInfo, physics)
	
	std::unordered_map<std::string, uint32_t> RenderableGeometryEntity::sbtOffsets {};
	
	// Buffer Write Lock
	RenderableGeometryEntity::BufferWriteLock::BufferWriteLock() : lock({}), valid(false) {}
	RenderableGeometryEntity::BufferWriteLock::BufferWriteLock(std::recursive_mutex mu, bool valid) : lock(mu), valid(valid) {}
	RenderableGeometryEntity::BufferWriteLock::BufferWriteLock(std::unique_lock<std::recursive_mutex> lock, bool valid) : lock(std::move(lock)), valid(valid) {}
	RenderableGeometryEntity::BufferWriteLock::operator bool() const {return valid;}
	void RenderableGeometryEntity::BufferWriteLock::Unlock() {
		if (valid) {
			valid = false;
			lock.unlock();
		}
	}
	RenderableGeometryEntity::BufferWriteLock RenderableGeometryEntity::GetBuffersWriteLock() {
		std::unique_lock<std::recursive_mutex> lock(writeMutex);
		if (device) {
			return BufferWriteLock(std::move(lock), true);
		}
		return BufferWriteLock();
	}
	
	void RenderableGeometryEntity::FreeComponentsBuffers() {
		std::unique_lock<std::recursive_mutex> lock(writeMutex);
		if (device) {
			meshGeometries.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshIndices16.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshIndices32.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			proceduralVertexAABB.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexPosition.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexNormal.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexColorU8.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexColorU16.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexColorF32.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexUV.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshCustomData.Do([this](auto& component){
				component.FreeBuffers(device);
			});
		}
		generated = false;
		blas = nullptr;
		device = nullptr;
	}
	
	void RenderableGeometryEntity::PushComponents(Device* device, VkCommandBuffer commandBuffer) {
		meshGeometriesComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshIndices16Components.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshIndices32Components.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexPositionComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		proceduralVertexAABBComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexNormalComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexColorU8Components.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexColorU16Components.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexColorF32Components.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexUVComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshCustomDataComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
	}
	
	void RenderableGeometryEntity::operator()(v4d::modular::ModuleID moduleId, uint64_t objId) {
		entityInstanceInfo.moduleVen = moduleId.vendor;
		entityInstanceInfo.moduleId = moduleId.module;
		entityInstanceInfo.objId = objId;
	}
	
	void RenderableGeometryEntity::Allocate(Device* renderingDevice, std::string sbtOffset, int geometriesCount) {
		std::unique_lock<std::recursive_mutex> lock(writeMutex);
		this->device = renderingDevice;
		this->sbtOffset = sbtOffsets[sbtOffset];
		geometriesAccelerationStructureInfo.clear();
		if (geometriesCount > 0) {
			Add_meshGeometries()->AllocateBuffersCount(renderingDevice, geometriesCount);
			geometriesAccelerationStructureInfo.resize(geometriesCount);
		}
	}
	
	RenderableGeometryEntity* RenderableGeometryEntity::SetInitialTransform(const glm::dmat4& t) {
		worldTransform = t;
		return this;
	}
	
	void RenderableGeometryEntity::SetWorldTransform(glm::dmat4 t) {
		worldTransform = t;
	}
	
	glm::dmat4 RenderableGeometryEntity::GetWorldTransform() {
		return worldTransform;
	}
	
	RenderableGeometryEntity::~RenderableGeometryEntity() {
		generator = [](auto*,Device*){};
		FreeComponentsBuffers();
	}
	
	void RenderableGeometryEntity::Generate(Device* device) {
		assert(!generated);
		generated = true;
		geometries.clear();
		generator(this, device);
		if (generated && geometriesAccelerationStructureInfo.size() > 0) {
			// Geometries
			if (auto geometriesData = meshGeometries.Lock(); geometriesData && geometriesData->data) {
				geometriesData->dirtyOnDevice = true;
				
				// handle default single-geometry entities
				if (geometries.size() == 0 && geometriesAccelerationStructureInfo.size() == 1) {
					auto& geometry = geometries.emplace_back();
					// Indices 16
					if (auto indexData = meshIndices16.Lock(); indexData && indexData->data) {
						geometry.indexCount = indexData->count;
					} // Indices 32
					else if (auto indexData = meshIndices32.Lock(); indexData && indexData->data) {
						geometry.indexCount = indexData->count;
					}
					// Vertex Positions
					if (auto vertexData = meshVertexPosition.Lock(); vertexData && vertexData->data) {
						geometry.vertexCount = vertexData->count;
					} else if (auto proceduralVertexData = proceduralVertexAABB.Lock(); proceduralVertexData && proceduralVertexData->data) {
						geometry.vertexCount = proceduralVertexData->count;
					} else {
						LOG_ERROR("Geometry has no vertex positions or AABB")
						return;
					}
				}
				
				assert(geometries.size() == geometriesAccelerationStructureInfo.size());
				
				entityInstanceInfo.geometries = device->GetBufferDeviceAddress(geometriesData->deviceBuffer);
				
				size_t i = 0;
				for (auto& geometry : geometries) {
					geometriesData->data[i] = {};
					
					// Transform & Material
					geometriesData->data[i].transform = glm::mat3x4(glm::transpose(geometry.transform));
					geometriesData->data[i].material = geometry.material;
					if (geometry.transform != glm::mat4{1}) {
						geometriesAccelerationStructureInfo[i].transformBuffer = device->GetBufferDeviceOrHostAddressConst(geometriesData->deviceBuffer);
						geometriesAccelerationStructureInfo[i].transformOffset = geometriesData->TypeSize() * i;
					}
					
					// Indices 16
					if (auto indexData = meshIndices16.Lock(); indexData && indexData->data) {
						indexData->dirtyOnDevice = true;
						geometriesAccelerationStructureInfo[i].indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
						geometriesAccelerationStructureInfo[i].indexOffset = geometry.firstIndex16 * indexData->TypeSize();
						geometriesAccelerationStructureInfo[i].indexCount = geometry.indexCount;
						geometriesAccelerationStructureInfo[i].indexStride = indexData->TypeSize();

						geometriesData->data[i].indices16 = (uint64_t)geometriesAccelerationStructureInfo[i].indexBuffer.deviceAddress + (uint64_t)geometry.firstIndex16 * indexData->TypeSize();
					}
					// Indices 32
					else if (auto indexData = meshIndices32.Lock(); indexData && indexData->data) {
						indexData->dirtyOnDevice = true;
						geometriesAccelerationStructureInfo[i].indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
						geometriesAccelerationStructureInfo[i].indexOffset = geometry.firstIndex32 * indexData->TypeSize();
						geometriesAccelerationStructureInfo[i].indexCount = geometry.indexCount;
						geometriesAccelerationStructureInfo[i].indexStride = indexData->TypeSize();

						geometriesData->data[i].indices32 = (uint64_t)geometriesAccelerationStructureInfo[i].indexBuffer.deviceAddress + (uint64_t)geometry.firstIndex32 * indexData->TypeSize();
					}
					
					// Vertex Positions
					if (auto vertexData = meshVertexPosition.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesAccelerationStructureInfo[i].vertexBuffer = device->GetBufferDeviceOrHostAddressConst(vertexData->deviceBuffer);
						geometriesAccelerationStructureInfo[i].vertexOffset = geometry.firstVertexPosition * vertexData->TypeSize();
						geometriesAccelerationStructureInfo[i].vertexCount = geometry.vertexCount;
						geometriesAccelerationStructureInfo[i].vertexStride = vertexData->TypeSize();
						
						geometriesData->data[i].vertexPositions = (uint64_t)geometriesAccelerationStructureInfo[i].vertexBuffer.deviceAddress + (uint64_t)geometry.firstVertexAABB * vertexData->TypeSize();
					}
					// Procedural vertices AABB
					else if (auto proceduralVertexData = proceduralVertexAABB.Lock(); proceduralVertexData && proceduralVertexData->data) {
						proceduralVertexData->dirtyOnDevice = true;
						geometriesAccelerationStructureInfo[i].vertexBuffer = device->GetBufferDeviceOrHostAddressConst(proceduralVertexData->deviceBuffer);
						geometriesAccelerationStructureInfo[i].vertexOffset = geometry.firstVertexAABB * proceduralVertexData->TypeSize();
						geometriesAccelerationStructureInfo[i].vertexCount = geometry.vertexCount;
						geometriesAccelerationStructureInfo[i].vertexStride = proceduralVertexData->TypeSize();
						
						geometriesData->data[i].vertexPositions = (uint64_t)geometriesAccelerationStructureInfo[i].vertexBuffer.deviceAddress + (uint64_t)geometry.firstVertexAABB * proceduralVertexData->TypeSize();
					} else {
						LOG_ERROR("Geometry has no vertex positions or AABB")
						return;
					}
					
					// Vertex normals
					if (auto vertexData = meshVertexNormal.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesData->data[i].vertexNormals = (uint64_t)device->GetBufferDeviceAddress(vertexData->deviceBuffer) + (uint64_t)geometry.firstVertexNormal * vertexData->TypeSize();
					}
					
					// Vertex colors uint 8
					if (auto vertexData = meshVertexColorU8.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesData->data[i].vertexColorsU8 = (uint64_t)device->GetBufferDeviceAddress(vertexData->deviceBuffer) + (uint64_t)geometry.firstVertexColorU8 * vertexData->TypeSize();
					}
					// Vertex colors uint 16
					if (auto vertexData = meshVertexColorU16.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesData->data[i].vertexColorsU16 = (uint64_t)device->GetBufferDeviceAddress(vertexData->deviceBuffer) + (uint64_t)geometry.firstVertexColorU16 * vertexData->TypeSize();
					}
					// Vertex colors float 32
					if (auto vertexData = meshVertexColorF32.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesData->data[i].vertexColorsF32 = (uint64_t)device->GetBufferDeviceAddress(vertexData->deviceBuffer) + (uint64_t)geometry.firstVertexColorF32 * vertexData->TypeSize();
					}
					
					// Vertex UVs
					if (auto vertexData = meshVertexUV.Lock(); vertexData && vertexData->data) {
						vertexData->dirtyOnDevice = true;
						geometriesData->data[i].vertexUVs = (uint64_t)device->GetBufferDeviceAddress(vertexData->deviceBuffer) + (uint64_t)geometry.firstVertexUV * vertexData->TypeSize();
					}
					
					// Custom Data
					if (auto data = meshCustomData.Lock(); data && data->data) {
						data->dirtyOnDevice = true;
						geometriesData->data[i].customData = (uint64_t)device->GetBufferDeviceAddress(data->deviceBuffer) + (uint64_t)geometry.firstCustomData * data->TypeSize();
					}
					
					++i;
				}
			}
		}
	}
	
}
