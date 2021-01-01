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
		
		struct Material {
			uint8_t type = 0;
			uint8_t metallic = 0;
			uint8_t roughness = 255;
			uint8_t indexOfRefraction = (uint8_t)(1.45 * 50);
			glm::u8vec4 baseColor {255,255,255,255};
			glm::u8vec4 rim {0,0,0,0};
			float emission = 0;
			uint8_t textures[8] {0};
			uint8_t texFactors[8] {0};
			
			Material() {static_assert(sizeof(Material) == 32);}
		};
		
		struct Geometry {
			glm::mat4 transform {1};
			Material material {};
			std::string_view materialName {""};
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
		
		struct GeometryInfo {
			glm::mat4 transform {1};
			uint64_t indices16 {};
			uint64_t indices32 {};
			uint64_t vertexPositions {};
			uint64_t vertexNormals {};
			uint64_t vertexColorsU8 {};
			uint64_t vertexColorsU16 {};
			uint64_t vertexColorsF32 {};
			uint64_t vertexUVs {};
			uint64_t customData = 0;
			uint64_t extra = 0;
			Material material {};
			GeometryInfo() {static_assert(sizeof(GeometryInfo) == 176 && (sizeof(GeometryInfo) % 16) == 0);}
		};
		
		struct RenderableEntityInstance {
			glm::mat4 modelViewTransform {1};
			uint64_t moduleVen {0};
			uint64_t moduleId {0};
			uint64_t objId {0};
			uint64_t geometries {0};
			// for 128 bytes, we're missing two vec4 (velocity vectors ?)
			RenderableEntityInstance() {static_assert(sizeof(RenderableEntityInstance) == 96);}
		};
		
		struct SharedGeometryData {
			Blas blas {};
			uint64_t geometriesBuffer = 0;
			bool isRayTracedTriangles = false;
			bool isRayTracedProceduralAABB = false;
			std::vector<Geometry> geometries {};
			std::vector<v4d::graphics::vulkan::rtx::AccelerationStructure::GeometryAccelerationStructureInfo> geometriesAccelerationStructureInfo;
		};
		
	private:
		V4D_ENTITY_DECLARE_CLASS(RenderableGeometryEntity)
		
		V4D_ENTITY_DECLARE_COMPONENT(RenderableGeometryEntity, Mesh::DataBuffer<GeometryInfo>, meshGeometries)
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
		std::function<void(RenderableGeometryEntity*, Device*)> generator = [](RenderableGeometryEntity* entity, Device*){ entity->generated = false; };
		RenderableEntityInstance entityInstanceInfo {};
		std::recursive_mutex writeMutex;
		std::shared_ptr<SharedGeometryData> sharedGeometryData = nullptr;
		uint32_t sbtOffset = 0;
		uint32_t rayTracingMask = 0x01;
		VkGeometryInstanceFlagsKHR rayTracingFlags = 0;
		glm::dmat4 cameraOffset {1};
		
		bool raster_transparent = false;
		float raster_wireframe = 0;
		glm::vec4 raster_wireframe_color {0.7f};
		
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
		
		v4d::graphics::RenderableGeometryEntity::GeometryInfo* Allocate(Device* renderingDevice, std::string sbtOffset = "V4D_raytracing:default", int geometriesCount = 1);
		
		RenderableGeometryEntity* SetInitialTransform(const glm::dmat4&);
		void SetWorldTransform(glm::dmat4);
		glm::dmat4 GetWorldTransform();
		
		bool Generate(Device* device); // returns true if generated correctly with at least one shared geometry, false if either error or using existing geometry (we can use this return value to generate something only once per shared geometry and only if something was generated)
		
		~RenderableGeometryEntity();
		
	};

}
