#include <v4d.h>

namespace v4d::graphics {

	V4D_ENTITY_DEFINE_CLASS(RenderableGeometryEntity)
	
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ModelTransform>, transform);
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ProceduralVertexAABB>, proceduralVertexAABB)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexPosition>, meshVertexPosition)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexNormal>, meshVertexNormal)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor>, meshVertexColor)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexUV>, meshVertexUV)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index>, meshIndices)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<glm::f32>, customData)
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
			meshIndices.Do([this](auto& component){
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
			meshVertexColor.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			meshVertexUV.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			transform.Do([this](auto& component){
				component.FreeBuffers(device);
			});
			customData.Do([this](auto& component){
				component.FreeBuffers(device);
			});
		}
		generated = false;
		blas = nullptr;
		device = nullptr;
	}
	
	void RenderableGeometryEntity::PushComponents(Device* device, VkCommandBuffer commandBuffer) {
		meshIndicesComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
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
		meshVertexColorComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		meshVertexUVComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		transformComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& transform){
			if (entityInstanceIndex != -1) {
				transform.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
		customDataComponents.ForEach([device, commandBuffer](auto entityInstanceIndex, auto& data){
			if (entityInstanceIndex != -1) {
				data.Push(device, commandBuffer);
			} else LOG_ERROR("trying to push component of a destroyed entity")
		});
	}
	
	void RenderableGeometryEntity::operator()(v4d::modular::ModuleID moduleId, uint64_t objId) {
		modelInfo.moduleVen = moduleId.vendor;
		modelInfo.moduleId = moduleId.module;
		modelInfo.objId = objId;
	}
	
	void RenderableGeometryEntity::Allocate(Device* renderingDevice, std::string sbtOffset) {
		std::unique_lock<std::recursive_mutex> lock(writeMutex);
		this->device = renderingDevice;
		this->sbtOffset = sbtOffsets[sbtOffset];
		Add_transform()->AllocateBuffers(renderingDevice);
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
		generator(this, device);
		if (generated) {
			// Indices
			if (auto indexData = meshIndices.Lock(); indexData && indexData->data) {
				indexData->dirtyOnDevice = true;
				geometryData.indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
				geometryData.indexOffset = 0;
				geometryData.indexCount = indexData->count;
				geometryData.indexSize = sizeof(Mesh::Index);

				modelInfo.indices = geometryData.indexBuffer.deviceAddress;
			}
			// Vertex Positions
			if (auto vertexData = meshVertexPosition.Lock(); vertexData && vertexData->data) {
				vertexData->dirtyOnDevice = true;
				geometryData.vertexBuffer = device->GetBufferDeviceOrHostAddressConst(vertexData->deviceBuffer);
				geometryData.vertexOffset = 0;
				geometryData.vertexCount = vertexData->count;
				geometryData.vertexSize = sizeof(Mesh::VertexPosition);
				
				modelInfo.vertexPositions = geometryData.vertexBuffer.deviceAddress;
			} else if (auto proceduralVertexData = proceduralVertexAABB.Lock(); proceduralVertexData && proceduralVertexData->data) {
				proceduralVertexData->dirtyOnDevice = true;
				geometryData.vertexBuffer = device->GetBufferDeviceOrHostAddressConst(proceduralVertexData->deviceBuffer);
				geometryData.vertexOffset = 0;
				geometryData.vertexCount = proceduralVertexData->count;
				geometryData.vertexSize = sizeof(Mesh::ProceduralVertexAABB);
				
				modelInfo.vertexPositions = geometryData.vertexBuffer.deviceAddress;
			}
			// Vertex normals
			if (auto vertexData = meshVertexNormal.Lock(); vertexData && vertexData->data) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexNormals = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Vertex colors
			if (auto vertexData = meshVertexColor.Lock(); vertexData && vertexData->data) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexColors = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Vertex UVs
			if (auto vertexData = meshVertexUV.Lock(); vertexData && vertexData->data) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexUVs = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Transform
			if (auto transformData = transform.Lock(); transformData && transformData->data) {
				transformData->dirtyOnDevice = true;
				modelInfo.transform = device->GetBufferDeviceAddress(transformData->deviceBuffer);
			}
			// Custom Data
			if (auto data = customData.Lock(); data && data->data) {
				data->dirtyOnDevice = true;
				modelInfo.customData = device->GetBufferDeviceAddress(data->deviceBuffer);
			}
		}
	}
	
}
