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
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, RenderableGeometryEntity::LightSource, lightSource)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, v4d::scene::PhysicsInfo, physics)
		
	public:
		Device* device = nullptr;
		v4d::graphics::vulkan::rtx::AccelerationStructure::GeometryData geometryData;
		std::shared_ptr<Blas> blas = nullptr;
		bool generated = false;
		glm::dmat4 initialTransform = glm::dmat4{1};
		std::function<void(RenderableGeometryEntity*)> generator = [](auto*){};
		Mesh::ModelInfo modelInfo {};
		uint32_t sbtOffset = 0;
		uint32_t rayTracingMask = 0xff;
		VkGeometryInstanceFlagsKHR rayTracingFlags = 0;
		std::mutex writeMutex;
		
		bool raster_transparent = false;
		bool raster_wireframe = false;
		
		static std::unordered_map<std::string, uint32_t> sbtOffsets;
		
		class V4DLIB BufferWriteLock {
			std::unique_lock<std::mutex> lock;
			bool valid;
		public:
			BufferWriteLock();
			BufferWriteLock(std::mutex mu, bool valid);
			BufferWriteLock(std::unique_lock<std::mutex> lock, bool valid);
			operator bool() const;
			void Unlock();
		};
		
		BufferWriteLock GetBuffersWriteLock();
		
		void FreeComponentsBuffers();
		
		void operator()(v4d::modular::ModuleID moduleId, int objId = 0, int customData = 0);
		
		void Prepare(Device* renderingDevice, std::string sbtOffset = "default");
		RenderableGeometryEntity* SetInitialTransform(const glm::dmat4&);
		
		void Generate(Device* device);
		
		~RenderableGeometryEntity();
		
	};

}
