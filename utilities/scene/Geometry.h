#pragma once

#include <v4d.h>

#include "LightSource.hpp"
#include "geometry_attributes.hh"

namespace v4d::scene {
	using namespace v4d::graphics;
	
	struct GeometryRenderType {
		RasterShaderPipeline* rasterShader = nullptr;
		int32_t sbtOffset = -1;
	};

	struct ObjectInstance;
	
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
		
		#pragma region Rendering
			VkGeometryInstanceFlagsKHR flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			glm::vec3 custom3f = {0,0,0};
			glm::mat4 custom4x4f = glm::mat4{0};
			bool renderWireframe = false;
			glm::vec4 wireframeColor {1.0f,1.0f,1.0f,1.0f};
			float wireframeThickness = 1.0f;
			uint32_t rayTracingMask = 	GEOMETRY_ATTR_PRIMARY_VISIBLE|
										GEOMETRY_ATTR_CAST_SHADOWS|
										GEOMETRY_ATTR_REFLECTION_VISIBLE|
										GEOMETRY_ATTR_COLLIDER;
		#pragma endregion
		
		#pragma region State
			bool active = true;
			bool isDirty = false;
			float boundingDistance = 0.0f;
			glm::vec3 boundingBoxSize = {0,0,0};
		#pragma endregion
		
		#pragma region Physics
			
			// Reserved for use by V4D_Physics module class
			enum class ColliderType : int {
				NONE,
				SPHERE,
				BOX,
				TRIANGLE_MESH,
				STATIC_PLANE,
				HEIGHTFIELD
			};
			ColliderType colliderType = ColliderType::TRIANGLE_MESH;
			void* colliderShapeObject = nullptr;
			bool colliderDirty = true;
		
		#pragma endregion
	
		#pragma region Buffer offsets
			uint32_t geometryOffset = 0;
			uint32_t vertexOffset = 0;
			uint32_t indexOffset = 0;
		#pragma endregion

		#pragma region Smart Pointers
			std::shared_ptr<v4d::graphics::vulkan::rtx::AccelerationStructure> blas = nullptr;
			std::shared_ptr<Geometry> duplicateFrom = nullptr;
		#pragma endregion
		
		#pragma region Buffer Definitions

			struct ObjectBuffer_T { // 256 bytes
				glm::dmat4 modelTransform;
				glm::dvec4 custom4d_0;
				glm::dvec4 custom4d_1;
				glm::dvec4 custom4d_2;
				uint64_t moduleVen;
				uint64_t moduleId;
				uint64_t objId : 32;
				uint64_t flags : 32; // reserved for future use
				uint64_t customUInt64;
				ObjectBuffer_T() {static_assert(sizeof(ObjectBuffer_T) == 256);}
			};
			struct GeometryBuffer_T { // 256 bytes
				glm::u32 indexOffset;
				glm::u32 vertexOffset;
				glm::u32 objectIndex; //TODO could limit to 23 bits
				glm::u32 material;
				glm::mat4 modelTransform;
				glm::mat4 modelViewTransform;
				glm::mat3 normalViewTransform;
				glm::vec3 custom3f;
				glm::mat4 custom4x4f;
				GeometryBuffer_T() {static_assert(sizeof(GeometryBuffer_T) == 256);}
			};
			using IndexBuffer_T = glm::u32; // 4 bytes
			struct VertexBuffer_T { // 32 bytes (must be same as ProceduralVertexBuffer_T)
				glm::vec3 pos;
				glm::f32 _color;
				glm::vec3 normal;
				union {
					glm::f32 _uv;
					uint32_t customData;
				};
				VertexBuffer_T() {static_assert(sizeof(VertexBuffer_T) == 32);}
				
				void SetColor(const glm::vec4& rgba) {_color = PackColorAsFloat(rgba);}
				glm::vec4 GetColor() const {return UnpackColorFromFloat(_color);}
				void SetUV(const glm::vec2& st) {_uv = PackUVasFloat(st);}
				glm::vec2 GetUV() const {return UnpackUVfromFloat(_uv);}
				
				static std::vector<VertexInputAttributeDescription> GetInputAttributes() {
					return {
						{0, offsetof(VertexBuffer_T, pos), VK_FORMAT_R32G32B32_SFLOAT},
						{1, offsetof(VertexBuffer_T, _color), VK_FORMAT_R32_UINT},
						{2, offsetof(VertexBuffer_T, normal), VK_FORMAT_R32G32B32_SFLOAT},
						{3, offsetof(VertexBuffer_T, _uv), VK_FORMAT_R32_UINT},
					};
				}
				
				bool operator==(const VertexBuffer_T& other) const {
					return pos == other.pos && normal == other.normal && _color == other._color && customData == other.customData;
				}
			};
			struct ProceduralVertexBuffer_T { // 32 bytes (must be same as VertexBuffer_T)
				glm::vec3 aabbMin;
				glm::vec3 aabbMax;
				glm::f32 _color;
				glm::f32 custom1;
				ProceduralVertexBuffer_T() {static_assert(sizeof(ProceduralVertexBuffer_T) == 32);}
				
				void SetColor(const glm::vec4& rgba) {_color = PackColorAsFloat(rgba);}
				glm::vec4 GetColor() const {return UnpackColorFromFloat(_color);}
				
				static std::vector<VertexInputAttributeDescription> GetInputAttributes() {
					return {
						{0, offsetof(ProceduralVertexBuffer_T, aabbMin), VK_FORMAT_R32G32B32_SFLOAT},
						{1, offsetof(ProceduralVertexBuffer_T, aabbMax), VK_FORMAT_R32G32B32_SFLOAT},
						{2, offsetof(ProceduralVertexBuffer_T, _color), VK_FORMAT_R32_UINT},
						{3, offsetof(ProceduralVertexBuffer_T, custom1), VK_FORMAT_R32_SFLOAT},
					};
				}
			};
			struct LightBuffer_T { // 32 bytes
				glm::vec3 position;
				glm::f32 intensity;
				glm::u32 _colorAndType;
				glm::u32 attributes;
				glm::f32 radius;
				glm::f32 custom1;
				LightBuffer_T() {static_assert(sizeof(LightBuffer_T) == 32);}
				
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
			
		#pragma endregion
		
		static std::unordered_map<std::string, GeometryRenderType> geometryRenderTypes;

		template<class T, VkBufferUsageFlags U>
		struct GlobalBuffer : public StagedBuffer {
			GlobalBuffer(uint32_t initialBlocks) : StagedBuffer(U, sizeof(T) * initialBlocks, false) {}
			
			void Extend(uint32_t nBlocks) {
				ExtendSize(sizeof(T) * nBlocks);
			}
			
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
			
			void Extend(uint32_t nBlocks) {
				ExtendSize(sizeof(T) * nBlocks);
			}
		};

		typedef GlobalBuffer<ObjectBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> ObjectBuffer;
		typedef GlobalBuffer<GeometryBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> GeometryBuffer;
		#ifdef V4D_RENDERER_RAYTRACING_USE_DEVICE_LOCAL_VERTEX_INDEX_BUFFERS
			typedef GlobalBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR> IndexBuffer;
			typedef GlobalBuffer<VertexBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR> VertexBuffer;
		#else
			typedef GlobalHostBuffer<IndexBuffer_T, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR> IndexBuffer;
			typedef GlobalHostBuffer<VertexBuffer_T, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR> VertexBuffer;
		#endif
		typedef GlobalBuffer<LightBuffer_T, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT> LightBuffer;

		GeometryBuffer_T* geometryInfo = nullptr;
		IndexBuffer_T* indices = nullptr;
		VertexBuffer_T* vertices = nullptr;
		
		Buffer* transformBuffer = nullptr;
		VkDeviceSize transformOffset = 0;
		
		bool geometryInfoInitialized = false;
		
		std::vector<IndexBuffer_T> simplifiedMeshIndices {};
		
		class V4DLIB GlobalGeometryBuffers {
			std::mutex geometryBufferMutex, vertexBufferMutex, indexBufferMutex, objectBufferMutex, lightBufferMutex;
			std::map<int, ObjectInstance*> objectAllocations {};
			std::map<int, Geometry*> geometryAllocations {};
			std::map<int, GeometryBufferAllocation> indexAllocations {};
			std::map<int, GeometryBufferAllocation> vertexAllocations {};
			std::map<int, LightSource*> lightAllocations {};
			
			// about 57 mb total
			static const int nbInitialLights = 256; // 256 lights @ 32 bytes each = 8 kb
			static const int nbInitialObjects = 1024; // 1k objects @ 256 bytes each = 256 kb
			static const int nbInitialGeometries = nbInitialObjects * 2; // 2k geometries @ 256 bytes each = 512 kb
			static const int nbInitialVertices = nbInitialGeometries * 512; // 1 million @ 32 bytes each = 32 mb
			static const int nbInitialIndices = nbInitialVertices * 6; // 6 million @ 4 bytes each = 24 mb
		
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
			
			void ShrinkGeometryIndices(Geometry*, uint32_t newIndexCount);
			void ShrinkGeometryVertices(Geometry*, uint32_t newVertexCount);
			
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
			
			void PushGeometriesInfo(Device*, VkCommandBuffer);
			void PullGeometriesInfo(Device*, VkCommandBuffer);
			
			void DefragmentMemory();
		};

		static GlobalGeometryBuffers globalBuffers;
		
		Geometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material = 0, bool isProcedural = false);
		Geometry(std::shared_ptr<Geometry> duplicateFrom);
		
		~Geometry();
		
		void Reset(uint32_t vertexCount, uint32_t indexCount);
		
		void Shrink(uint32_t newVertexCount, uint32_t newIndexCount);
		
		void MapStagingBuffers();
		
		void UnmapStagingBuffers();
		
		void SetGeometryInfo(uint32_t objectIndex = 0, uint32_t material = 0);
		
		void SetGeometryTransform(glm::dmat4 modelTransform, const glm::dmat4& viewMatrix);
		
		void SetVertex(uint32_t i, const VertexBuffer_T&);
		void SetProceduralVertex(uint32_t i, const ProceduralVertexBuffer_T&);
		void SetVertex(uint32_t i, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color);
		void SetProceduralVertex(uint32_t i, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1 = 0);
		
		void SetIndex(uint32_t i, IndexBuffer_T vertexIndex);
		void SetIndices(const std::vector<IndexBuffer_T>& vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		void SetIndices(const IndexBuffer_T* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		//TODO void SetTriangle(uint32_t i, IndexBuffer_T v0, IndexBuffer_T v1, IndexBuffer_T v2) {}
		//TODO void GetTriangle(uint32_t i, IndexBuffer_T* v0, IndexBuffer_T* v1, IndexBuffer_T* v2) {}
		
		void GetGeometryInfo(uint32_t* objectIndex = nullptr, uint32_t* material = nullptr);
		
		void GetVertex(uint32_t i, glm::vec3* pos, glm::vec3* normal = nullptr, glm::vec2* uv = nullptr, glm::vec4* color = nullptr);
		
		VertexBuffer_T* GetVertexPtr(uint32_t i = 0);
		
		void GetProceduralVertex(uint32_t i, glm::vec3* aabbMin, glm::vec3* aabbMax, glm::vec4* color = nullptr, float* custom1 = nullptr);
		
		ProceduralVertexBuffer_T* GetProceduralVertexPtr(uint32_t i = 0);
		
		void GetIndex(uint32_t i, IndexBuffer_T* vertexIndex);
		
		IndexBuffer_T GetIndex(uint32_t i);
		
		IndexBuffer_T* GetIndexPtr(uint32_t i = 0);
		
		void GetIndices(std::vector<IndexBuffer_T>* vertexIndices, uint32_t count = 0, uint32_t startAt = 0);
		
		void AutoPush(Device* device, VkCommandBuffer commandBuffer, bool forcePushTransform = true);
		
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

namespace std {
	template<> struct hash<v4d::scene::Geometry::VertexBuffer_T> {
		size_t operator()(v4d::scene::Geometry::VertexBuffer_T const &vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::f32>()(vertex._color) << 1)) >> 1) ^
					(hash<glm::vec3>()(vertex.normal) << 1);
		}
	};
}
