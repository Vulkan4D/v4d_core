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
		
		struct Geometry {
			glm::mat4 transform {1};
			uint64_t material = 0;
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			uint32_t firstIndex = 0;
			uint32_t firstVertexPosition = 0;
			uint32_t firstVertexAABB = 0;
			uint32_t firstVertexNormal = 0;
			uint32_t firstVertexColorU8 = 0;
			uint32_t firstVertexColorU16 = 0;
			uint32_t firstVertexColorF32 = 0;
			uint32_t firstVertexUV = 0;
			uint32_t firstCustomData = 0;
		};
		
		struct SharedGeometryData {
			Blas blas {};
			uint64_t geometriesBuffer = 0;
			std::vector<Geometry> geometries {};
			std::vector<v4d::graphics::vulkan::rtx::AccelerationStructure::GeometryAccelerationStructureInfo> geometriesAccelerationStructureInfo;
		};
		
	private:
		V4D_ENTITY_DECLARE_CLASS(RenderableGeometryEntity)
		
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::GeometryInfo>, meshGeometries)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index16>, meshIndices16)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::Index32>, meshIndices32)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::ProceduralVertexAABB>, proceduralVertexAABB)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexPosition>, meshVertexPosition)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexNormal>, meshVertexNormal)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<uint8_t>>, meshVertexColorU8)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<uint16_t>>, meshVertexColorU16)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexColor<glm::f32>>, meshVertexColorF32)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<Mesh::VertexUV>, meshVertexUV)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<glm::f32>, meshCustomData)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, RenderableGeometryEntity::LightSource, lightSource)
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, v4d::scene::PhysicsInfo, physics)
		
	public:
		Device* device = nullptr;
		bool generated = false;
		glm::dmat4 worldTransform = glm::dmat4{1};
		std::function<void(RenderableGeometryEntity*, Device*)> generator = [](RenderableGeometryEntity*, Device*){};
		Mesh::RenderableEntityInstance entityInstanceInfo {};
		std::recursive_mutex writeMutex;
		std::shared_ptr<SharedGeometryData> sharedGeometryData = nullptr;
		uint32_t sbtOffset = 0;
		uint32_t rayTracingMask = 0xff;
		VkGeometryInstanceFlagsKHR rayTracingFlags = 0;
		
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
		
		void Allocate(Device* renderingDevice, std::string sbtOffset = "default", int geometriesCount = 1);
		
		RenderableGeometryEntity* SetInitialTransform(const glm::dmat4&);
		void SetWorldTransform(glm::dmat4);
		glm::dmat4 GetWorldTransform();
		
		void Generate(Device* device);
		
		~RenderableGeometryEntity();
		
	};

}
