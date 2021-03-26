#include "RenderableGeometryEntity.h"
#include "utilities/io/Logger.h"

namespace v4d::graphics {

	V4D_ENTITY_DEFINE_CLASS(RenderableGeometryEntity)
	
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<RenderableGeometryEntity::GeometryInfo>, meshGeometries)
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
			sharedGeometryData.reset();
		}
		generated = false;
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
	
	v4d::graphics::RenderableGeometryEntity::GeometryInfo* RenderableGeometryEntity::Allocate(Device* renderingDevice, std::string sbtOffset, int geometriesCount) {
		std::unique_lock<std::recursive_mutex> lock(writeMutex);
		device = renderingDevice;
		this->sbtOffset = Renderer::sbtOffsets[std::string("rendering:hit:")+sbtOffset];
		if (geometriesCount > 0) {
			std::vector<v4d::graphics::RenderableGeometryEntity::GeometryInfo> list (geometriesCount);
			return Add_meshGeometries()->AllocateBuffersFromVector(renderingDevice, list);
		}
		return nullptr;
	}
	
	RenderableGeometryEntity* RenderableGeometryEntity::SetInitialTransform(const glm::dmat4& t, std::shared_ptr<RenderableGeometryEntity> parent) {
		this->parent = parent;
		SetLocalTransform(t);
		return this;
	}
	
	void RenderableGeometryEntity::SetLocalTransform(const glm::dmat4& t) {
		transformMatrix = t;
	}
	
	void RenderableGeometryEntity::SetWorldTransform(const glm::dmat4& t) {
		transformMatrix = glm::inverse(GetParentWorldTransform()) * t;
	}
	
	glm::dmat4 RenderableGeometryEntity::GetWorldTransform() const {
		return GetParentWorldTransform() * transformMatrix;
	}
	
	glm::dmat4& RenderableGeometryEntity::GetLocalTransform() {
		return transformMatrix;
	}
	
	glm::dmat4 RenderableGeometryEntity::GetParentWorldTransform() const {
		if (parent)  {
			return parent->GetWorldTransform();
		}
		return glm::dmat4{1};
	}
	
	RenderableGeometryEntity::~RenderableGeometryEntity() {
		generator = [](auto*,Device*){};
		FreeComponentsBuffers();
	}
	
	bool RenderableGeometryEntity::Generate(Device* device) {
		assert(!generated);
		generated = true;
		generator(this, device);
		
		if (generated) {
			if (sharedGeometryData && sharedGeometryData->geometriesBuffer) {
				entityInstanceInfo.geometries = sharedGeometryData->geometriesBuffer;
				return false;
			} else {
				// Geometries
				if (auto geometriesData = meshGeometries.Lock(); geometriesData && geometriesData->data && geometriesData->count > 0) {
					if (!sharedGeometryData) sharedGeometryData = std::make_shared<SharedGeometryData>();
					sharedGeometryData->geometriesAccelerationStructureInfo.resize(geometriesData->count);
					geometriesData->dirtyOnDevice = true;
					
					// handle default single-geometry entities
					if (sharedGeometryData->geometries.size() == 0 && sharedGeometryData->geometriesAccelerationStructureInfo.size() == 1) {
						auto& geometry = sharedGeometryData->geometries.emplace_back();
						// Material
						geometry.material = geometriesData->data->material;
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
							return false;
						}
					}
					
					assert(sharedGeometryData->geometries.size() == sharedGeometryData->geometriesAccelerationStructureInfo.size());
					
					sharedGeometryData->geometriesBuffer = entityInstanceInfo.geometries = device->GetBufferDeviceAddress(geometriesData->deviceBuffer);
					
					size_t i = 0;
					for (auto& geometry : sharedGeometryData->geometries) {
						geometriesData->data[i] = {};
						
						// Transform & Material
						geometriesData->data[i].transform = glm::mat3x4(glm::transpose(geometry.transform));
						geometriesData->data[i].material = geometry.material;
						if (geometry.transform != glm::mat4{1}) {
							sharedGeometryData->geometriesAccelerationStructureInfo[i].transformBuffer = device->GetBufferDeviceOrHostAddressConst(geometriesData->deviceBuffer);
							sharedGeometryData->geometriesAccelerationStructureInfo[i].transformOffset = geometriesData->TypeSize() * i;
						}
						
						// Indices 16
						if (auto indexData = meshIndices16.Lock(); indexData && indexData->data) {
							indexData->dirtyOnDevice = true;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexOffset = geometry.firstIndex * indexData->TypeSize();
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexCount = geometry.indexCount;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexStride = indexData->TypeSize();

							geometriesData->data[i].indices16 = (uint64_t)sharedGeometryData->geometriesAccelerationStructureInfo[i].indexBuffer.deviceAddress + (uint64_t)geometry.firstIndex * indexData->TypeSize();
						}
						// Indices 32
						else if (auto indexData = meshIndices32.Lock(); indexData && indexData->data) {
							indexData->dirtyOnDevice = true;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexOffset = geometry.firstIndex * indexData->TypeSize();
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexCount = geometry.indexCount;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].indexStride = indexData->TypeSize();

							geometriesData->data[i].indices32 = (uint64_t)sharedGeometryData->geometriesAccelerationStructureInfo[i].indexBuffer.deviceAddress + (uint64_t)geometry.firstIndex * indexData->TypeSize();
						}
						
						// Vertex Positions
						if (auto vertexData = meshVertexPosition.Lock(); vertexData && vertexData->data) {
							vertexData->dirtyOnDevice = true;
							sharedGeometryData->isRayTracedTriangles = true;
							sharedGeometryData->isRayTracedProceduralAABB = false;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexBuffer = device->GetBufferDeviceOrHostAddressConst(vertexData->deviceBuffer);
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexOffset = geometry.firstVertexPosition * vertexData->TypeSize();
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexCount = geometry.vertexCount;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexStride = vertexData->TypeSize();
							
							geometriesData->data[i].vertexPositions = (uint64_t)sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexBuffer.deviceAddress + (uint64_t)geometry.firstVertexAABB * vertexData->TypeSize();
						}
						// Procedural vertices AABB
						else if (auto proceduralVertexData = proceduralVertexAABB.Lock(); proceduralVertexData && proceduralVertexData->data) {
							proceduralVertexData->dirtyOnDevice = true;
							sharedGeometryData->isRayTracedTriangles = false;
							sharedGeometryData->isRayTracedProceduralAABB = true;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexBuffer = device->GetBufferDeviceOrHostAddressConst(proceduralVertexData->deviceBuffer);
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexOffset = geometry.firstVertexAABB * proceduralVertexData->TypeSize();
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexCount = geometry.vertexCount;
							sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexStride = proceduralVertexData->TypeSize();
							
							geometriesData->data[i].vertexPositions = (uint64_t)sharedGeometryData->geometriesAccelerationStructureInfo[i].vertexBuffer.deviceAddress + (uint64_t)geometry.firstVertexAABB * proceduralVertexData->TypeSize();
						} else {
							LOG_ERROR("Geometry has no vertex positions or AABB")
							return false;
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
					return sharedGeometryData->geometries.size() > 0;
				}
			}
		}
		return false;
	}
	
	std::shared_ptr<RenderableGeometryEntity> RenderableGeometryEntity::GetRoot(std::shared_ptr<RenderableGeometryEntity>& entity) {
		if (!entity) return nullptr;
		if (!entity->parent || entity->parent == entity) return entity;
		return GetRoot(entity->parent);
	}
	
}
