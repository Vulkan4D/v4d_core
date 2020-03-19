#pragma once

#include <v4d.h>

namespace v4d::graphics {
	
	#pragma region Pack Helpers
	
	/////////////////
	// NOT WORKING
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
	/////////////////

	V4DLIB glm::f32 PackColorAsFloat(glm::vec4 color);
	V4DLIB glm::u32 PackColorAsUint(glm::vec4 color);
	V4DLIB glm::vec4 UnpackColorFromFloat(glm::f32 color);
	V4DLIB glm::vec4 UnpackColorFromUint(glm::u32 color);
	V4DLIB glm::f32 PackUVasFloat(glm::vec2 uv);
	V4DLIB glm::u32 PackUVasUint(glm::vec2 uv);
	V4DLIB glm::vec2 UnpackUVfromFloat(glm::f32 uv);
	V4DLIB glm::vec2 UnpackUVfromUint(glm::u32 uv);

	#pragma endregion

	struct ObjectInstance;
	
	struct LightSource {
		glm::dvec3 position {0};
		glm::f32 intensity = 0;
		glm::vec3 color {1};
		glm::u32 type = 0;
		glm::u32 attributes = 0;
		glm::f32 radius = 0;
		glm::f32 custom1 = 0;
		
		uint32_t lightOffset = 0;
		glm::vec3 viewSpacePosition {0};
		
		LightSource (
			glm::dvec3 position = {0,0,0},
			glm::f32 intensity = 0,
			glm::vec3 color = {1,1,1},
			glm::u32 type = 0,
			glm::u32 attributes = 0,
			glm::f32 radius = 0,
			glm::f32 custom1 = 0
		) : 
			position(position),
			intensity(intensity),
			color(color),
			type(type),
			attributes(attributes),
			radius(radius),
			custom1(custom1)
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
		uint32_t material;
		bool isProcedural;
		
		uint32_t geometryOffset = 0;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		struct GeometryBuffer_T { // 16 bytes
			glm::u32 indexOffset;
			glm::u32 vertexOffset;
			glm::u32 objectIndex;
			glm::u32 material;
		};
		using IndexBuffer_T = glm::u32; // 4 bytes
		struct VertexBuffer_T { // 32 bytes (must be same as ProceduralVertexBuffer_T)
			glm::vec3 pos;
			glm::f32 _color;
			glm::vec3 normal;
			glm::f32 _uv;
			
			void SetColor(const glm::vec4& rgba) {_color = PackColorAsFloat(rgba);}
			glm::vec4 GetColor() const {return UnpackColorFromFloat(_color);}
			void SetUV(const glm::vec2& st) {_uv = PackUVasFloat(st);}
			glm::vec2 GetUV() const {return UnpackUVfromFloat(_uv);}
		};
		struct ProceduralVertexBuffer_T { // 32 bytes (must be same as VertexBuffer_T)
			glm::vec3 aabbMin;
			glm::vec3 aabbMax;
			glm::f32 _color;
			glm::f32 custom1;
			
			void SetColor(const glm::vec4& rgba) {_color = PackColorAsFloat(rgba);}
			glm::vec4 GetColor() const {return UnpackColorFromFloat(_color);}
		};
		struct LightBuffer_T { // 32 bytes
			glm::vec3 position;
			glm::f32 intensity;
			glm::u32 _colorAndType;
			glm::u32 attributes;
			glm::f32 radius;
			glm::f32 custom1;
			
			void SetColorAndType(glm::vec3 color, glm::u8 type) {
				_colorAndType = PackColorAsUint(glm::vec4(color, 0));
				_colorAndType |= type & 0x000000ff;
			}
			void GetColorAndType(glm::vec3* color = nullptr, glm::u8* type = nullptr) const {
				if (color) *color = glm::vec3(UnpackColorFromUint(_colorAndType));
				if (type) *type = _colorAndType & 0x000000ff;
			}
			void GetColorAndType(glm::vec3* color = nullptr, glm::u32* type = nullptr) const {
				if (color) *color = glm::vec3(UnpackColorFromUint(_colorAndType));
				if (type) *type = _colorAndType & 0x000000ff;
			}
		};
		
		struct ObjectBuffer_T { // 128 bytes
			glm::mat4 modelViewMatrix;
			glm::mat3 normalMatrix;
			glm::vec3 custom3;
			glm::vec4 custom4;
		};
	
		static std::unordered_map<std::string, uint32_t> rayTracingShaderOffsets;

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

		GeometryBuffer_T* geometryInfo = nullptr;
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
			
			// // Very high object and vertex count, takes about two seconds just for allocating 496 MB of VRAM
			// 	static const int nbInitialObjects = 1048576; // 1 million @ 128 bytes each = 128 mb
			// 	static const int nbInitialGeometries = 1048576; // 1 million @ 16 bytes each = 16 mb
			// 	static const int nbInitialVertices = 8388608; // 8 million @ 32 bytes each = 256 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 16 million @ 4 bytes each = 64 mb
			// 	static const int nbInitialLights = 1048576; // 1 million @ 32 bytes each = 32 mb
			
			// // Medium object and vertex count, takes about one second to allocate 148 MB of VRAM
			// 	static const int nbInitialObjects = 16384; // 16k objects @ 128 bytes each = 2 mb
			// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 64k @ 16 bytes each = 1 mb
			// 	static const int nbInitialVertices = 4194304; // 4 million @ 32 bytes each = 128 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 8 million @ 4 bytes each = 16 mb
			// 	static const int nbInitialLights = 16384; // 16k lights @ 32 bytes each = 512 kb
			
			// // Low object and vertex count, allocates almost instantaneously 38 MB of VRAM
			// 	static const int nbInitialObjects = 8192; // 8k objects @ 128 bytes each = 1 mb
			// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 32k @ 16 bytes each = 512 kb
			// 	static const int nbInitialVertices = 1048576; // 1 million @ 32 bytes each = 32 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 2; // 2 million @ 4 bytes each = 4 mb
			// 	static const int nbInitialLights = 8192; // 8k lights @ 32 bytes each = 256 kb
			
			// Very Low object and vertex count, allocates instantaneously 10 MB of VRAM
				static const int nbInitialObjects = 4096; // 4k objects @ 128 bytes each = 512 kb
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
		
		Geometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material = 0, bool isProcedural = false);
		// Geometry(glm::vec3 spherePosition, float sphereRadius, uint32_t material = 0);
		
		~Geometry();
		
		VkGeometryNV GetRayTracingGeometry() const;
		
		void MapStagingBuffers();
		
		void UnmapStagingBuffers();
		
		void SetGeometryInfo(uint32_t objectIndex = 0, uint32_t material = 0);
		
		void SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color);
		
		void SetProceduralVertex(uint32_t i, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1 = 0);
		
		void SetIndex(uint32_t i, IndexBuffer_T vertexIndex);
		
		void SetIndices(const std::vector<IndexBuffer_T>& vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		//TODO void SetTriangle(uint32_t i, IndexBuffer_T v0, IndexBuffer_T v1, IndexBuffer_T v2) {}
		//TODO void GetTriangle(uint32_t i, IndexBuffer_T* v0, IndexBuffer_T* v1, IndexBuffer_T* v2) {}
		
		void SetIndices(const IndexBuffer_T* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		void GetGeometryInfo(uint32_t* objectIndex = nullptr, uint32_t* material = nullptr);
		
		void GetVertex(uint32_t i, glm::vec3* pos, glm::vec3* normal = nullptr, glm::vec2* uv = nullptr, glm::vec4* color = nullptr);
		
		VertexBuffer_T* GetVertexPtr(uint32_t i);
		
		void GetProceduralVertex(uint32_t i, glm::vec3* aabbMin, glm::vec3* aabbMax, glm::vec4* color = nullptr, float* custom1 = nullptr);
		
		ProceduralVertexBuffer_T* GetProceduralVertexPtr(uint32_t i);
		
		void GetIndex(uint32_t i, IndexBuffer_T* vertexIndex);
		
		IndexBuffer_T GetIndex(uint32_t i);
		
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

}
