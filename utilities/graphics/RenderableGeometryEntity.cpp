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
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, RenderableGeometryEntity::LightSource, lightSource)
	V4D_ENTITY_DEFINE_COMPONENT(RenderableGeometryEntity, v4d::scene::PhysicsInfo, physics)
	
	std::unordered_map<std::string, uint32_t> RenderableGeometryEntity::sbtOffsets {};

	void RenderableGeometryEntity::FreeComponentsBuffers() {
		if (device) {
			transform.Do([this](auto& component){
				if (component.data) initialTransform = component.data->worldTransform;
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
			meshIndices.Do([this](auto& component){
				component.FreeBuffers(device);
			});
		}
		generated = false;
		blas = nullptr;
	}
	
	void RenderableGeometryEntity::operator()(v4d::modular::ModuleID moduleId, int objId, int customData) {
		modelInfo.moduleVen = moduleId.vendor;
		modelInfo.moduleId = moduleId.module;
		modelInfo.objId = objId;
		modelInfo.customData = customData;
	}
	
	void RenderableGeometryEntity::Prepare(Device* renderingDevice, std::string sbtOffset) {
		this->device = renderingDevice;
		Add_transform();
		transform->AllocateBuffers(renderingDevice);
		this->sbtOffset = sbtOffsets[sbtOffset];
		auto t = transform.Lock();
		if (t->data) {
			t->data->worldTransform = initialTransform;
		}
	}
	
	RenderableGeometryEntity* RenderableGeometryEntity::SetInitialTransform(const glm::dmat4& t) {
		initialTransform = t;
		return this;
	}
	
	RenderableGeometryEntity::~RenderableGeometryEntity() {
		FreeComponentsBuffers();
		generator = [](auto*){};
	}
	
	void RenderableGeometryEntity::Generate(Device* device) {
		generated = true;
		generator(this);
		if (generated) {
			// Indices
			if (auto indexData = meshIndices.Lock(); indexData) {
				indexData->dirtyOnDevice = true;
				geometryData.indexBuffer = device->GetBufferDeviceOrHostAddressConst(indexData->deviceBuffer);
				geometryData.indexOffset = 0;
				geometryData.indexCount = indexData->count;
				geometryData.indexSize = sizeof(Mesh::Index);

				modelInfo.indices = geometryData.indexBuffer.deviceAddress;
			}
			// Vertex Positions
			if (auto vertexData = meshVertexPosition.Lock(); vertexData) {
				vertexData->dirtyOnDevice = true;
				geometryData.vertexBuffer = device->GetBufferDeviceOrHostAddressConst(vertexData->deviceBuffer);
				geometryData.vertexOffset = 0;
				geometryData.vertexCount = vertexData->count;
				geometryData.vertexSize = sizeof(Mesh::VertexPosition);
				
				modelInfo.vertexPositions = geometryData.vertexBuffer.deviceAddress;
			} else if (auto proceduralVertexData = proceduralVertexAABB.Lock(); proceduralVertexData) {
				proceduralVertexData->dirtyOnDevice = true;
				geometryData.vertexBuffer = device->GetBufferDeviceOrHostAddressConst(proceduralVertexData->deviceBuffer);
				geometryData.vertexOffset = 0;
				geometryData.vertexCount = proceduralVertexData->count;
				geometryData.vertexSize = sizeof(Mesh::ProceduralVertexAABB);
				
				modelInfo.vertexPositions = geometryData.vertexBuffer.deviceAddress;
			}
			// Vertex normals
			if (auto vertexData = meshVertexNormal.Lock(); vertexData) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexNormals = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Vertex colors
			if (auto vertexData = meshVertexColor.Lock(); vertexData) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexColors = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Vertex UVs
			if (auto vertexData = meshVertexUV.Lock(); vertexData) {
				vertexData->dirtyOnDevice = true;
				modelInfo.vertexUVs = device->GetBufferDeviceAddress(vertexData->deviceBuffer);
			}
			// Transform
			if (auto transformData = transform.Lock(); transformData) {
				transformData->dirtyOnDevice = true;
				modelInfo.transform = device->GetBufferDeviceAddress(transformData->deviceBuffer);
			}
		}
	}
	
}
