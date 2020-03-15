#pragma once

#include <v4d.h>

namespace v4d::graphics {
	
	struct ObjectInstance;
	
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
		uint32_t materialIndex;
		
		uint32_t geometryOffset = 0;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		using GeometryBuffer_T = glm::uvec4; // 16 bytes
		using IndexBuffer_T = 	glm::u32; // 4 bytes
		struct VertexBuffer_T { // 32 bytes
			glm::vec4 posAndUV;
			glm::vec4 normalAndColor;
		};
		struct LightBuffer_T { // 32 bytes
			alignas(16) glm::vec3 position;
			alignas(4) glm::f32 intensity;
			alignas(4) glm::u32 colorAndType;
			alignas(4) glm::u32 attributes;
			alignas(4) glm::f32 radius;
			alignas(4) glm::f32 _unused_;
		};
		struct ObjectBuffer_T { // 64 bytes
			alignas(64) glm::mat4 modelViewMatrix;
			// alignas(64) glm::mat3 normalMatrix; // This does not work... need to do more debugging...
		};
	

		#pragma region Pack Helpers
		
		// static NormalBuffer_T PackNormal(glm::vec3 normal) {
		// 	// // vec2
		// 	// float f = glm::sqrt(8.0f * normal.z + 8.0f);
		// 	// return glm::vec2(normal) / f + 0.5f;
			
		// 	// vec4
		// 	return glm::vec4(normal, 0);
		// }

		// static glm::vec3 UnpackNormal(NormalBuffer_T norm) {
		// 	// glm::vec2 fenc = norm * 4.0f - 2.0f;
		// 	// float f = glm::dot(fenc, fenc);
		// 	// float g = glm::sqrt(1.0f - f / 4.0f);
		// 	// return glm::vec3(fenc * g, 1.0f - f / 2.0f);
		// 	return norm;
		// }

		static glm::f32 PackColorAsFloat(glm::vec4 color) {
			color *= 255.0f;
			glm::uvec4 pack {
				glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
			};
			return glm::uintBitsToFloat((pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a);
		}

		static glm::u32 PackColorAsUint(glm::vec4 color) {
			color *= 255.0f;
			glm::uvec4 pack {
				glm::clamp(glm::u32(color.r), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.g), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.b), (glm::u32)0, (glm::u32)255),
				glm::clamp(glm::u32(color.a), (glm::u32)0, (glm::u32)255),
			};
			return (pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a;
		}

		static glm::vec4 UnpackColorFromFloat(glm::f32 color) {
			glm::u32 packed = glm::floatBitsToUint(color);
			return glm::vec4(
				(packed & 0xff000000) >> 24,
				(packed & 0x00ff0000) >> 16,
				(packed & 0x0000ff00) >> 8,
				(packed & 0x000000ff) >> 0
			) / 255.0f;
		}
		
		static glm::vec4 UnpackColorFromUint(glm::u32 color) {
			return glm::vec4(
				(color & 0xff000000) >> 24,
				(color & 0x00ff0000) >> 16,
				(color & 0x0000ff00) >> 8,
				(color & 0x000000ff) >> 0
			) / 255.0f;
		}
		
		static glm::f32 PackUVasFloat(glm::vec2 uv) {
			uv *= 65535.0f;
			glm::uvec2 pack {
				glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
				glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
			};
			return glm::uintBitsToFloat((pack.s << 16) | pack.t);
		}

		static glm::u32 PackUVasUint(glm::vec2 uv) {
			uv *= 65535.0f;
			glm::uvec2 pack {
				glm::clamp(glm::u32(uv.s), (glm::u32)0, (glm::u32)65535),
				glm::clamp(glm::u32(uv.t), (glm::u32)0, (glm::u32)65535),
			};
			return (pack.s << 16) | pack.t;
		}

		static glm::vec2 UnpackUVfromFloat(glm::f32 uv) {
			glm::u32 packed = glm::floatBitsToUint(uv);
			return glm::vec2(
				(packed & 0xffff0000) >> 16,
				(packed & 0x0000ffff) >> 0
			) / 65535.0f;
		}

		static glm::vec2 UnpackUVfromUint(glm::u32 uv) {
			return glm::vec2(
				(uv & 0xffff0000) >> 16,
				(uv & 0x0000ffff) >> 0
			) / 65535.0f;
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

		typedef GlobalBuffer<ObjectBuffer_T> ObjectBuffer;
		typedef GlobalBuffer<GeometryBuffer_T> GeometryBuffer;
		typedef GlobalBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> IndexBuffer;
		typedef GlobalBuffer<VertexBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> VertexBuffer;
		typedef GlobalBuffer<LightBuffer_T> LightBuffer;

		GeometryBuffer_T* geometries = nullptr;
		IndexBuffer_T* indices = nullptr;
		VertexBuffer_T* vertices = nullptr;
		
		Buffer* transformBuffer = nullptr;
		VkDeviceSize transformOffset = 0;
		
		bool geometryInfoInitialized = false;
		
		class V4DLIB GlobalGeometryBuffers {
			std::mutex geometryBufferMutex, vertexBufferMutex, indexBufferMutex, objectBufferMutex, lightBufferMutex;
			std::map<int, ObjectInstance*> objectAllocations {};
			std::map<int, Geometry*> geometryAllocations {};
			std::map<int, GeometryBufferAllocation> indexAllocations {};
			std::map<int, GeometryBufferAllocation> vertexAllocations {};
			std::map<int, LightSource*> lightAllocations {};
			
			// // Very high object and vertex count, takes about two seconds just for allocating 432 MB of VRAM
			// 	static const int nbInitialObjects = 1048576; // 1 million @ 64 bytes each = 64 mb
			// 	static const int nbInitialGeometries = 1048576; // 1 million @ 16 bytes each = 16 mb
			// 	static const int nbInitialVertices = 8388608; // 8 million @ 32 bytes each = 256 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 16 million @ 4 bytes each = 64 mb
			// 	static const int nbInitialLights = 1048576; // 1 million @ 32 bytes each = 32 mb
			
			// // Medium object and vertex count, takes about one second to allocate 147 MB of VRAM
			// 	static const int nbInitialObjects = 16384; // 16k objects @ 64 bytes each = 1 mb
			// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 64k @ 16 bytes each = 1 mb
			// 	static const int nbInitialVertices = 4194304; // 4 million @ 32 bytes each = 128 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 8 million @ 4 bytes each = 16 mb
			// 	static const int nbInitialLights = 16384; // 16k lights @ 32 bytes each = 512 kb
			
			// // Low object and vertex count, allocates almost instantaneously 38 MB of VRAM
			// 	static const int nbInitialObjects = 8192; // 8k objects @ 64 bytes each = 512 kb
			// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 32k @ 16 bytes each = 512 kb
			// 	static const int nbInitialVertices = 1048576; // 1 million @ 32 bytes each = 32 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 2 million @ 4 bytes each = 4 mb
			// 	static const int nbInitialLights = 8192; // 8k lights @ 32 bytes each = 256 kb
			
			// Very Low object and vertex count, allocates instantaneously 10 MB of VRAM
				static const int nbInitialObjects = 4096; // 4k objects @ 64 bytes each = 256 kb
				static const int nbInitialGeometries = nbInitialObjects * 4; // 16k @ 16 bytes each = 256 kb
				static const int nbInitialVertices = 262144; // 262k vertices @ 32 bytes each = 8 mb
				static const int nbInitialIndices = nbInitialVertices * 2; // 524k indices @ 4 bytes each = 1 mb
				static const int nbInitialLights = 4096; // 4k lights @ 32 bytes each = 128 kb
			
		public:
			
			enum GeometryBuffersMask : uint8_t {
				BUFFER_GEOMETRY_INFO = 0x01,
				BUFFER_INDEX = 0x02,
				BUFFER_VERTEX = 0x04,
				BUFFER_ALL = 0xff,
			};
			
			ObjectBuffer objectBuffer {nbInitialObjects};
			GeometryBuffer geometryBuffer {nbInitialGeometries};
			IndexBuffer indexBuffer {nbInitialIndices};
			VertexBuffer vertexBuffer {nbInitialVertices};
			LightBuffer lightBuffer {nbInitialLights};
			
			uint32_t nbAllocatedObjects = 0;
			uint32_t nbAllocatedGeometries = 0;
			uint32_t nbAllocatedIndices = 0;
			uint32_t nbAllocatedVertices = 0;
			uint32_t nbAllocatedLights = 0;
			
			int/*geometryOffset*/ AddGeometry(Geometry*);
			int/*objectOffset*/ AddObject(ObjectInstance*);
			int/*lightOffset*/ AddLight(LightSource*);
			
			void RemoveGeometry(Geometry*);
			void RemoveObject(ObjectInstance*);
			void RemoveLight(LightSource*);
			
			void Allocate(Device*);
			void Free(Device*);
			
			void PushEverything(Device*, VkCommandBuffer);
			void PullEverything(Device*, VkCommandBuffer);
			
			void PushAllGeometries(Device*, VkCommandBuffer);
			void PullAllGeometries(Device*, VkCommandBuffer);
			
			void PushGeometry(Device*, VkCommandBuffer, Geometry*, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL, 
								uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
								uint32_t indexCount = 0, uint32_t indexOffset = 0
			);
			void PullGeometry(Device*, VkCommandBuffer, Geometry*, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL, 
								uint32_t vertexCount = 0, uint32_t vertexOffset = 0,
								uint32_t indexCount = 0, uint32_t indexOffset = 0
			);
			
			void WriteObject(ObjectInstance*);
			void ReadObject(ObjectInstance*);
			
			void WriteLight(LightSource*);
			void ReadLight(LightSource*);
			
			void PushLights(Device*, VkCommandBuffer);
			void PullLights(Device*, VkCommandBuffer);
			
			void PushObjects(Device*, VkCommandBuffer);
			void PullObjects(Device*, VkCommandBuffer);
			
			void DefragmentMemory();
		};

		static GlobalGeometryBuffers globalBuffers;
		
		VkGeometryNV GetRayTracingGeometry() const;
		
		Geometry(uint32_t vertexCount, uint32_t indexCount, uint32_t materialIndex = 0);
		
		~Geometry();
		
		void MapStagingBuffers();
		
		void UnmapStagingBuffers();
		
		void SetGeometryInfo(uint32_t objectIndex = 0, uint32_t materialIndex = 0);
		
		void SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color);
		
		void SetIndex(uint32_t i, IndexBuffer_T vertexIndex);
		
		void SetIndices(const std::vector<IndexBuffer_T>& vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		//TODO void SetTriangle(uint32_t i, IndexBuffer_T v0, IndexBuffer_T v1, IndexBuffer_T v2) {}
		//TODO void GetTriangle(uint32_t i, IndexBuffer_T* v0, IndexBuffer_T* v1, IndexBuffer_T* v2) {}
		
		void SetIndices(const IndexBuffer_T* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		void GetGeometryInfo(uint32_t* objectIndex, uint32_t* materialIndex);
		
		void GetVertex(uint32_t i, glm::vec3* pos, glm::vec3* normal, glm::vec2* uv, glm::vec4* color);
		
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
