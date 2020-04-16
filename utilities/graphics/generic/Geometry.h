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
		
		enum : uint32_t {
			RAY_TRACING_TYPE_SOLID = 0x01,
			RAY_TRACING_TYPE_LIQUID = 0x02,
			RAY_TRACING_TYPE_CLOUD = 0x04,
			RAY_TRACING_TYPE_PARTICLE = 0x08,
			RAY_TRACING_TYPE_TRANSPARENT = 0x10,
			RAY_TRACING_TYPE_CUTOUT = 0x20,
			RAY_TRACING_TYPE_CELESTIAL = 0x40,
			RAY_TRACING_TYPE_EMITTER = 0x80,
		};
		uint32_t rayTracingMask = RAY_TRACING_TYPE_SOLID;
		VkGeometryInstanceFlagsKHR flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		
		uint32_t geometryOffset = 0;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;
		
		bool active = true;
		bool isDirty = false;
		
		float boundingDistance = 0.0f;
		
		std::shared_ptr<v4d::graphics::vulkan::rtx::AccelerationStructure> blas = nullptr;
		std::shared_ptr<Geometry> duplicateFrom = nullptr;

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
			
			static std::vector<VertexInputAttributeDescription> GetInputAttributes() {
				return {
					// {0, offsetof(VertexBuffer_T, pos), VK_FORMAT_R32G32B32A32_SFLOAT},
					// {1, offsetof(VertexBuffer_T, normal), VK_FORMAT_R32G32B32A32_SFLOAT},
					
					{0, offsetof(VertexBuffer_T, pos), VK_FORMAT_R32G32B32_SFLOAT},
					{1, offsetof(VertexBuffer_T, _color), VK_FORMAT_R32_UINT},
					{2, offsetof(VertexBuffer_T, normal), VK_FORMAT_R32G32B32_SFLOAT},
					{3, offsetof(VertexBuffer_T, _uv), VK_FORMAT_R32_UINT},
				};
			}
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
		
		struct ObjectBuffer_T { // 256 bytes
			glm::mat4 modelMatrix;
			glm::mat4 custom4x4;
			glm::mat4 modelViewMatrix;
			glm::mat3 normalMatrix;
			glm::vec3 custom3;
			glm::vec4 custom4;
		};
	
		static std::unordered_map<std::string, uint32_t> rayTracingShaderOffsets;

		template<class T, VkBufferUsageFlags U>
		struct GlobalBuffer : public StagedBuffer {
			GlobalBuffer(uint32_t initialBlocks) : StagedBuffer(U, sizeof(T) * initialBlocks, false) {}
			
			void Push(Device* device, VkCommandBuffer commandBuffer, int count, int offset = 0) {
				Buffer::Copy(device, commandBuffer, stagingBuffer, deviceLocalBuffer, sizeof(T) * count, sizeof(T) * offset, sizeof(T) * offset);
			}
			
			void Pull(Device* device, VkCommandBuffer commandBuffer, int count, int offset = 0) {
				Buffer::Copy(device, commandBuffer, deviceLocalBuffer, stagingBuffer, sizeof(T) * count, sizeof(T) * offset, sizeof(T) * offset);
			}
		};

		template<class T, VkBufferUsageFlags U>
		struct GlobalHostBuffer : public Buffer {
			GlobalHostBuffer(uint32_t initialBlocks) : Buffer(U, sizeof(T) * initialBlocks, false) {}
		};

		typedef GlobalBuffer<ObjectBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> ObjectBuffer;
		typedef GlobalBuffer<GeometryBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> GeometryBuffer;
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			typedef GlobalBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT> IndexBuffer;
			typedef GlobalBuffer<VertexBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT> VertexBuffer;
		#else
			typedef GlobalHostBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT> IndexBuffer;
			typedef GlobalHostBuffer<VertexBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT> VertexBuffer;
		#endif
		typedef GlobalBuffer<LightBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> LightBuffer;

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
			
				// // Ultra High object and vertex count, takes about 10 seconds to allocate 3376 MB of memory
				// 	static const int nbInitialObjects = 1048576; // 1 million @ 256 bytes each = 256 mb
				// 	static const int nbInitialGeometries = 1048576; // 1 million @ 16 bytes each = 16 mb
				// 	static const int nbInitialVertices = 67108864; // 67 million @ 32 bytes each = 2048 mb
				// 	static const int nbInitialIndices = nbInitialVertices * 4; // 270 million @ 4 bytes each = 1024 mb
				// 	static const int nbInitialLights = 1048576; // 1 million @ 32 bytes each = 32 mb
				
				// // Very High vertex count, takes about 5 seconds to allocate 1556 MB of memory
				// 	static const int nbInitialObjects = 65536; // 65k objects @ 256 bytes each = 16 mb
				// 	static const int nbInitialGeometries = 131072; // 131k geometries @ 16 bytes each = 2 mb
				// 	static const int nbInitialVertices = 33554432; // 33 million @ 32 bytes each = 1024 mb
				// 	static const int nbInitialIndices = nbInitialVertices * 4; // 134 million @ 4 bytes each = 512 mb
				// 	static const int nbInitialLights = 65536; // 65k lights @ 32 bytes each = 2 mb
			
			// High object and vertex count, takes about 1 second to allocate 661 MB of memory
				static const int nbInitialObjects = 65536; // 65k objects @ 256 bytes each = 16 mb
				static const int nbInitialGeometries = 131072; // 131k geometries @ 16 bytes each = 2 mb
				static const int nbInitialVertices = 12000000; // 12 million @ 32 bytes each = 366 mb
				static const int nbInitialIndices = nbInitialVertices * 6; // 72 million @ 4 bytes each = 275 mb
				static const int nbInitialLights = 65536; // 65k lights @ 32 bytes each = 2 mb
			
				// // Medium object and vertex count, takes less than one second to allocate 230 MB of memory
				// 	static const int nbInitialObjects = 16384; // 16k objects @ 256 bytes each = 4 mb
				// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 64k @ 16 bytes each = 1 mb
				// 	static const int nbInitialVertices = 4194304; // 4 million @ 32 bytes each = 128 mb
				// 	static const int nbInitialIndices = nbInitialVertices * 6; // 24 million @ 4 bytes each = 96 mb
				// 	static const int nbInitialLights = 16384; // 16k lights @ 32 bytes each = 512 kb
				
				// // Low object and vertex count, allocates almost instantaneously 48 MB of memory
				// 	static const int nbInitialObjects = 8192; // 8k objects @ 256 bytes each = 1 mb
				// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 32k @ 16 bytes each = 1 mb
				// 	static const int nbInitialVertices = 1048576; // 1 million @ 32 bytes each = 32 mb
				// 	static const int nbInitialIndices = nbInitialVertices * 3; // 3 million @ 4 bytes each = 12 mb
				// 	static const int nbInitialLights = 8192; // 8k lights @ 32 bytes each = 256 kb
			
			// // Very Low object and vertex count, allocates instantaneously 12 MB of memory
			// 	static const int nbInitialObjects = 4096; // 4k objects @ 256 bytes each = 512 kb
			// 	static const int nbInitialGeometries = nbInitialObjects * 4; // 16k @ 16 bytes each = 512 kb
			// 	static const int nbInitialVertices = 262144; // 262k vertices @ 32 bytes each = 8 mb
			// 	static const int nbInitialIndices = nbInitialVertices * 3; // 786k indices @ 4 bytes each = 3 mb
			// 	static const int nbInitialLights = 4096; // 4k lights @ 32 bytes each = 128 kb
			
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
			
			void Allocate(Device*, const std::vector<uint32_t>& queueFamilies = {});
			void Free(Device*);
			
			void PushEverything(Device*, VkCommandBuffer);
			void PullEverything(Device*, VkCommandBuffer);
			
			void PushAllGeometries(Device*, VkCommandBuffer);
			void PullAllGeometries(Device*, VkCommandBuffer);
			
			void PushGeometry(Device*, VkCommandBuffer, Geometry*, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL
								#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
									, uint32_t vertexCount = 0, uint32_t vertexOffset = 0
									, uint32_t indexCount = 0, uint32_t indexOffset = 0
								#endif
			);
			void PullGeometry(Device*, VkCommandBuffer, Geometry*, 
								GeometryBuffersMask geometryBuffersMask = BUFFER_ALL
								#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
									, uint32_t vertexCount = 0, uint32_t vertexOffset = 0
									, uint32_t indexCount = 0, uint32_t indexOffset = 0
								#endif
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
		Geometry(std::shared_ptr<Geometry> duplicateFrom);
		
		~Geometry();
		
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
		
		void AutoPush(Device* device, VkCommandBuffer commandBuffer);
		
		void Push(Device* device, VkCommandBuffer commandBuffer, 
				GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask = GlobalGeometryBuffers::BUFFER_ALL
				#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
					, uint32_t vertexCount = 0, uint32_t vertexOffset = 0
					, uint32_t indexCount = 0, uint32_t indexOffset = 0
				#endif
			);

		void Pull(Device* device, VkCommandBuffer commandBuffer, 
				GlobalGeometryBuffers::GeometryBuffersMask geometryBuffersMask = GlobalGeometryBuffers::BUFFER_ALL
				#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
					, uint32_t vertexCount = 0, uint32_t vertexOffset = 0
					, uint32_t indexCount = 0, uint32_t indexOffset = 0
				#endif
			);

	};

}
