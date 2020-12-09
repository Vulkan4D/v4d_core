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
	
}
