#pragma once
#include <v4d.h>

namespace v4d::graphics {

	using Blas = v4d::graphics::vulkan::rtx::AccelerationStructure;

	class V4DLIB RenderableGeometryEntity {
	public:
		struct V4DLIB LightSource {
			glm::vec3 position;
			float radius;
			glm::vec3 color;
			float intensity;
			LightSource() {
				static_assert(sizeof(LightSource) == 32);
				Reset();
			}
			LightSource(glm::vec3 position, glm::vec3 color, float radius, float intensity) : 
				position(position),
				radius(radius),
				color(color),
				intensity(intensity)
			{}
			void Reset() {
				position = glm::vec3(0);
				radius = 0;
				color = glm::vec3(0);
				intensity = 0;
			}
		};
	private:
		V4D_ENTITY_DECLARE_CLASS(RenderableGeometryEntity)
		
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ModelTransform>, transform)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ProceduralVertexAABB>, proceduralVertexAABB)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexPosition>, meshVertexPosition)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexNormal>, meshVertexNormal)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor>, meshVertexColor)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexUV>, meshVertexUV)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index>, meshIndices)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<glm::f32>, customData)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, RenderableGeometryEntity::LightSource, lightSource)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, v4d::scene::PhysicsInfo, physics)
		
	public:
		Device* device = nullptr;
		v4d::graphics::vulkan::rtx::AccelerationStructure::GeometryData geometryData;
		std::shared_ptr<Blas> blas = nullptr;
		bool generated = false;
		glm::dmat4 worldTransform = glm::dmat4{1};
		std::function<void(RenderableGeometryEntity*, Device*)> generator = [](RenderableGeometryEntity*, Device*){};
		Mesh::ModelInfo modelInfo {};
		uint32_t sbtOffset = 0;
		uint32_t rayTracingMask = 0xff;
		VkGeometryInstanceFlagsKHR rayTracingFlags = 0;
		std::recursive_mutex writeMutex;
		
		bool raster_transparent = false;
		float raster_wireframe = 0;
		glm::vec4 raster_wireframe_color {0.7f};
		
		static std::unordered_map<std::string, uint32_t> sbtOffsets;
		
		class V4DLIB BufferWriteLock {
			std::unique_lock<std::recursive_mutex> lock;
			bool valid;
		public:
			BufferWriteLock();
			BufferWriteLock(std::recursive_mutex mu, bool valid);
			BufferWriteLock(std::unique_lock<std::recursive_mutex> lock, bool valid);
			operator bool() const;
			void Unlock();
		};
		
		BufferWriteLock GetBuffersWriteLock();
		
		void FreeComponentsBuffers();
		
		static void PushComponents(Device*, VkCommandBuffer);
		
		void operator()(v4d::modular::ModuleID moduleId, uint64_t objId = 0);
		
		void Allocate(Device* renderingDevice, std::string sbtOffset = "default");
		
		RenderableGeometryEntity* SetInitialTransform(const glm::dmat4&);
		void SetWorldTransform(glm::dmat4);
		glm::dmat4 GetWorldTransform();
		
		void Generate(Device* device);
		
		~RenderableGeometryEntity();
		
	};

}
