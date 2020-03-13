#pragma once

#include <v4d.h>

namespace v4d::graphics {
	
	struct LightSource {
		glm::dvec3 position {0};
		glm::f32 intensity = 0;
		glm::vec3 color {1};
		glm::u32 type = 0;
		glm::u32 attributes = 0;
		glm::f32 radius = 0;
		
		uint32_t lightOffset = 0;
		glm::vec3 viewSpacePosition {0};
		
		LightSource (
			glm::dvec3 position = {0,0,0},
			glm::f32 intensity = 0,
			glm::vec3 color = {1,1,1},
			glm::u32 type = 0,
			glm::u32 attributes = 0,
			glm::f32 radius = 0
		) : 
			position(position),
			intensity(intensity),
			color(color),
			type(type),
			attributes(attributes),
			radius(radius)
 		{}
	};
	
	class V4DLIB Geometry {
		struct GeometryBufferAllocation {
			uint32_t n;
			Geometry* data;
		};

	public:
		uint32_t vertexCount;
		uint32_t indexCount;
		
		uint32_t geometryOffset = 0;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		using GeometryBuffer_T = glm::uvec4;
		using IndexBuffer_T = 	glm::u32;
		using PosBuffer_T = 	glm::vec4;
		using MaterialBuffer_T = glm::u32;
		using NormalBuffer_T = 	glm::vec2;
		using UVBuffer_T = 		glm::u32;
		using ColorBuffer_T = 	glm::u32;
		struct LightBuffer_T {
			glm::vec3 position;
			glm::f32 intensity;
			glm::u32 colorAndType;
			glm::u32 attributes;
			glm::f32 radius;
			glm::f32 _unused_;
		};

		#pragma region Pack Helpers
		
		static NormalBuffer_T PackNormal(glm::vec3 normal) {
			float f = glm::sqrt(8.0f * normal.z + 8.0f);
			return glm::vec2(normal) / f + 0.5f;
		}

		static ColorBuffer_T PackColor(glm::vec4 color) {
			color *= 255.0f;
			glm::uvec4 pack {
				glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
			};
			return (pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a;
		}

		static UVBuffer_T PackUV(glm::vec2 uv) {
			uv *= 65535.0f;
			glm::uvec2 pack {
				glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
				glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
			};
			return (pack.s << 16) | pack.t;
		}

		static glm::vec3 UnpackNormal(NormalBuffer_T norm) {
			glm::vec2 fenc = norm * 4.0f - 2.0f;
			float f = glm::dot(fenc, fenc);
			float g = glm::sqrt(1.0f - f / 4.0f);
			return glm::vec3(fenc * g, 1.0f - f / 2.0f);
		}

		static glm::vec2 UnpackUV(UVBuffer_T uv) {
			return glm::vec2(
				(uv & 0xffff0000) >> 16,
				(uv & 0x0000ffff) >> 0
			) / 65535.0f;
		}

		static glm::vec4 UnpackColor(ColorBuffer_T color) {
			return glm::vec4(
				(color & 0xff000000) >> 24,
				(color & 0x00ff0000) >> 16,
				(color & 0x0000ff00) >> 8,
				(color & 0x000000ff) >> 0
			) / 255.0f;
		}
		
		#pragma endregion

		template<class T, VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT>
		struct GlobalBuffer : public StagedBuffer {
			GlobalBuffer(uint32_t initialBlocks) : StagedBuffer(usage, sizeof(T) * initialBlocks, false) {}
			
			void Push(Device* device, VkCommandBuffer commandBuffer, int count, int offset = 0) {
				Buffer::Copy(device, commandBuffer, stagingBuffer, deviceLocalBuffer, sizeof(T) * count, sizeof(T) * offset, sizeof(T) * offset);
			}
			
			void Pull(Device* device, VkCommandBuffer commandBuffer, int count, int offset = 0) {
				Buffer::Copy(device, commandBuffer, deviceLocalBuffer, stagingBuffer, sizeof(T) * count, sizeof(T) * offset, sizeof(T) * offset);
			}
		};

		typedef GlobalBuffer<GeometryBuffer_T> GeometryBuffer;
		typedef GlobalBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> IndexBuffer;
		typedef GlobalBuffer<PosBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> PosBuffer;
		typedef GlobalBuffer<MaterialBuffer_T> MaterialBuffer;
		typedef GlobalBuffer<NormalBuffer_T> NormalBuffer;
		typedef GlobalBuffer<UVBuffer_T> UVBuffer;
		typedef GlobalBuffer<ColorBuffer_T> ColorBuffer;
		typedef GlobalBuffer<LightBuffer_T> LightBuffer;

		GeometryBuffer_T* geometryInfo = nullptr;
		IndexBuffer_T* indices = nullptr;
		PosBuffer_T* vertexPositions = nullptr;
		MaterialBuffer_T* vertexMaterials = nullptr;
		NormalBuffer_T* vertexNormals = nullptr;
		UVBuffer_T* vertexUVs = nullptr;
		ColorBuffer_T* vertexColors = nullptr;
		
		Buffer* transformBuffer = nullptr;
		VkDeviceSize transformOffset = 0;
		
		bool geometryInfoInitialized = false;
		
		class V4DLIB GlobalGeometryBuffers {
			std::mutex geometryBufferMutex, vertexBufferMutex, indexBufferMutex, lightBufferMutex;
			std::map<int, Geometry*> geometryAllocations {};
			std::map<int, GeometryBufferAllocation> indexAllocations {};
			std::map<int, GeometryBufferAllocation> vertexAllocations {};
			std::map<int, LightSource*> lightAllocations {};
			
			static const int nbInitialGeometries = 1024;
			static const int nbInitialVertices = 1000000;
			static const int nbInitialIndices = nbInitialVertices * 2;
			static const int nbInitialLightSources = 4096;
			
		public:
			
			enum GeometryBuffersMask : uint8_t {
				BUFFER_GEOMETRY_INFO = 0x01,
				BUFFER_INDEX = 0x02,
				BUFFER_POS = 0x04,
				BUFFER_MAT = 0x08,
				BUFFER_NORM = 0x10,
				BUFFER_UV = 0x20,
				BUFFER_COLOR = 0x40,
				// BUFFER_*** = 0x80,
				BUFFER_ALL = 0xff,
				BUFFER_VERTEX_DATA = BUFFER_POS | BUFFER_MAT | BUFFER_NORM | BUFFER_UV | BUFFER_COLOR,
			};
			
			GeometryBuffer geometryBuffer {nbInitialGeometries};
			IndexBuffer indexBuffer {nbInitialIndices};
			PosBuffer posBuffer {nbInitialVertices};
			MaterialBuffer materialBuffer {nbInitialVertices};
			NormalBuffer normalBuffer {nbInitialVertices};
			UVBuffer uvBuffer {nbInitialVertices};
			ColorBuffer colorBuffer {nbInitialVertices};
			LightBuffer lightBuffer {nbInitialLightSources};
			
			uint32_t nbAllocatedGeometries = 0;
			uint32_t nbAllocatedIndices = 0;
			uint32_t nbAllocatedVertices = 0;
			uint32_t nbAllocatedLights = 0;
			
			int/*geometryOffset*/ AddGeometry(Geometry* geometry);
			int/*lightOffset*/ AddLight(LightSource* lightSource);
			
			void RemoveGeometry(Geometry* geometry);
			void RemoveLight(LightSource* lightSource);
			
			void Allocate(Device* device);
			void Free(Device* device);
			
			void PushAllGeometries(Device* device, VkCommandBuffer commandBuffer);
			void PullAllGeometries(Device* device, VkCommandBuffer commandBuffer);
			
			void PushGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL, 
								uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
								uint32_t indexCount = 0, uint32_t indexOffset = 0
			);
			void PullGeometry(Device* device, VkCommandBuffer commandBuffer, Geometry* geometry, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL, 
								uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
								uint32_t indexCount = 0, uint32_t indexOffset = 0
			);
			
			void WriteLightSource(LightSource* lightSource);
			void ReadLightSource(LightSource* lightSource);
			
			void PushLightSources(Device* device, VkCommandBuffer commandBuffer);
			void PullLightSources(Device* device, VkCommandBuffer commandBuffer);
			
			void DefragmentMemory();
		};

		static GlobalGeometryBuffers globalBuffers;
		
		VkGeometryNV GetRayTracingGeometry() const;
		
		Geometry(uint32_t vertexCount, uint32_t indexCount);
		
		~Geometry();
		
		void MapStagingBuffers();
		
		void UnmapStagingBuffers();
		
		void SetGeometryInfo(uint32_t objectIndex = 0, uint32_t otherIndex = 0);
		
		void SetVertex(uint32_t i, const glm::vec4& pos, MaterialBuffer_T material, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color);
		
		void SetIndex(uint32_t i, IndexBuffer_T vertexIndex);
		
		void SetIndices(const std::vector<IndexBuffer_T>& vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		//TODO void SetTriangle(uint32_t i, IndexBuffer_T v0, IndexBuffer_T v1, IndexBuffer_T v2) {}
		//TODO void GetTriangle(uint32_t i, IndexBuffer_T* v0, IndexBuffer_T* v1, IndexBuffer_T* v2) {}
		
		void SetIndices(const IndexBuffer_T* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		void GetGeometryInfo(uint32_t* objectIndex, uint32_t* otherIndex);
		
		void GetVertex(uint32_t i, glm::vec4* pos, glm::u32* material, glm::vec3* normal, glm::vec2* uv, glm::vec4* color);
		
		void GetIndex(uint32_t i, IndexBuffer_T* vertexIndex);
		
		void GetIndices(std::vector<IndexBuffer_T>* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		void Push(Device* device, VkCommandBuffer commandBuffer, 
				GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask = GlobalGeometryBuffers::BUFFER_ALL, 
				uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
				uint32_t indexCount = 0, uint32_t indexOffset = 0
			);

		void Pull(Device* device, VkCommandBuffer commandBuffer, 
				GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask = GlobalGeometryBuffers::BUFFER_ALL, 
				uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
				uint32_t indexCount = 0, uint32_t indexOffset = 0
			);

	};






	// struct ProceduralGeometryData {
	// 	std::pair<glm::vec3, glm::vec3> boundingBox;
	// 	ProceduralGeometryData(glm::vec3 min, glm::vec3 max) : boundingBox(min, max) {}
	// };
	// 	std::vector<V, std::allocator<V>> aabbData {};
	// 	Buffer* aabbBuffer = nullptr;
	// 	VkDeviceSize aabbOffset = 0;
	// 	VkDeviceSize aabbCount = 0;
	// 	VkGeometryNV GetRayTracingGeometry() const override {
	// 		if (aabbBuffer->buffer == VK_NULL_HANDLE)
	// 			throw std::runtime_error("Buffer for aabb geometry has not been created");
	// 		VkGeometryNV geometry {};
	// 		geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	// 		geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	// 		geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	// 		geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
	// 		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
	// 		geometry.geometry.aabbs.numAABBs = (uint)aabbData.size();
	// 		geometry.geometry.aabbs.stride = sizeof(V);
	// 		geometry.geometry.aabbs.offset = aabbOffset;
	// 		geometry.geometry.aabbs.aabbData = aabbBuffer->buffer;
	// 		return geometry;
	// 	}
	// 	ProceduralGeometry(const std::vector<V>& aabbData, Buffer* aabbBuffer, VkDeviceSize aabbOffset = 0)
	// 	 : aabbData(aabbData) {
	// 		this->aabbBuffer = aabbBuffer;
	// 		this->aabbOffset = aabbOffset;
	// 		this->aabbCount = aabbData.size();
	// 	}
	// };




}
