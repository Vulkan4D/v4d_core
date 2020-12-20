#pragma once

#include <v4d.h>

namespace v4d::graphics::Mesh {

	typedef uint16_t Index16;
	typedef uint32_t Index32;
	
	struct VertexPosition {
		glm::f32 x;
		glm::f32 y;
		glm::f32 z;
		VertexPosition(float x, float y, float z) : x(x), y(y), z(z) {}
		VertexPosition(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
		operator glm::vec3() {return glm::vec3(x, y, z);}
		VertexPosition() {static_assert(sizeof(VertexPosition) == 12);}
		static std::vector<VertexInputAttributeDescription> GetInputAttributes() {
			return {
				{0, offsetof(VertexPosition, x), VK_FORMAT_R32G32B32_SFLOAT},
			};
		}
		bool operator==(const VertexPosition& other) const {
			return x == other.x && y == other.y && z == other.z;
		}
		bool operator!=(const VertexPosition& other) const {return !(*this == other);}
	};
	
	struct VertexNormal {
		glm::f32 x;
		glm::f32 y;
		glm::f32 z;
		VertexNormal(float x, float y, float z) : x(x), y(y), z(z) {}
		VertexNormal(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
		operator glm::vec3() {return glm::vec3(x, y, z);}
		VertexNormal() {static_assert(sizeof(VertexNormal) == 12);}
		bool operator==(const VertexNormal& other) const {
			return x == other.x && y == other.y && z == other.z;
		}
		bool operator!=(const VertexNormal& other) const {return !(*this == other);}
	};
	
	template<typename T>
	struct VertexColor {
		T r;
		T g;
		T b;
		T a;
		VertexColor() : r(0), g(0), b(0), a(0) {}
		VertexColor(T r, T g, T b, T a) : r(r), g(g), b(b), a(a) {}
		VertexColor(const glm::vec<4, T>& v) : r(v.r), g(v.g), b(v.b), a(v.a) {}
		operator glm::vec<4, T>() {return glm::vec<4, T>(r, g, b, a);}
		bool operator==(const VertexColor& other) const {
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}
		bool operator!=(const VertexColor& other) const {return !(*this == other);}
	};
	using VertexColorU8 = VertexColor<uint8_t>;
	using VertexColorU16 = VertexColor<uint16_t>;
	using VertexColorF32 = VertexColor<glm::f32>;
	
	struct VertexUV {
		glm::f32 s;
		glm::f32 t;
		VertexUV(float s, float t) : s(s), t(t) {}
		VertexUV(const glm::vec2& v) : s(v.s), t(v.t) {}
		operator glm::vec2() {return glm::vec2(s, t);}
		VertexUV() {static_assert(sizeof(VertexUV) == 8);}
		bool operator==(const VertexUV& other) const {
			return s == other.s && t == other.t;
		}
		bool operator!=(const VertexUV& other) const {return !(*this == other);}
	};
	
	struct ProceduralVertexAABB {
		glm::vec3 aabbMin;
		glm::vec3 aabbMax;
		ProceduralVertexAABB(glm::vec3 aabbMin, glm::vec3 aabbMax) : aabbMin(aabbMin), aabbMax(aabbMax) {}
		ProceduralVertexAABB() {static_assert(sizeof(ProceduralVertexAABB) == 24);}
		static std::vector<VertexInputAttributeDescription> GetInputAttributes() {
			return {
				{0, offsetof(ProceduralVertexAABB, aabbMin), VK_FORMAT_R32G32B32_SFLOAT},
				{1, offsetof(ProceduralVertexAABB, aabbMax), VK_FORMAT_R32G32B32_SFLOAT},
			};
		}
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
		uint64_t material = 0;
		GeometryInfo() {static_assert(sizeof(GeometryInfo) == 144 && (sizeof(GeometryInfo) % 16) == 0);}
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
	
	template<typename T>
	struct DataBuffer {
		T* data = nullptr;
		size_t count = 0;
		VkBuffer hostBuffer = VK_NULL_HANDLE;
		VkBuffer deviceBuffer = VK_NULL_HANDLE;
		MemoryAllocation hostBufferAllocation = VK_NULL_HANDLE;
		MemoryAllocation deviceBufferAllocation = VK_NULL_HANDLE;
		bool dirtyOnDevice = false;
		
		size_t TypeSize() const {return sizeof(T);}
		
		T* AllocateBuffers(Device* device, const std::initializer_list<T>& list) {
			AllocateBuffersCount(device, list.size());
			memcpy(data, list.begin(), list.size() * sizeof(T));
			dirtyOnDevice = true;
			return data;
		}
		T* AllocateBuffers(Device* device, const std::vector<T>& list) {
			AllocateBuffersCount(device, list.size());
			memcpy(data, list.data(), list.size() * sizeof(T));
			dirtyOnDevice = true;
			return data;
		}
		T* AllocateBuffers(Device* device, T* inputDataOrArray, size_t elementCount = 1) {
			AllocateBuffersCount(device, elementCount);
			memcpy(data, inputDataOrArray, elementCount * sizeof(T));
			dirtyOnDevice = true;
			return data;
		}
		template<typename _T>
		T* AllocateBuffers(Device* device, _T&& value) {
			AllocateBuffersCount(device, 1);
			*data = std::forward<_T>(value);
			dirtyOnDevice = true;
			return data;
		}
		T* AllocateBuffers(Device* device) {
			return AllocateBuffersCount(device, 1);
		}
		T* AllocateBuffersCount(Device* device, uint32_t elementCount) {
			assert(elementCount > 0);
			
			count = elementCount;
			
			assert(!data);
			assert(!hostBuffer && !deviceBuffer);
			assert(!hostBufferAllocation && !deviceBufferAllocation);
			
			// Host buffer
			{VkBufferCreateInfo createInfo {};
				createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				createInfo.size = count * sizeof(T);
				createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				device->CreateAndAllocateBuffer(createInfo, MemoryUsage::MEMORY_USAGE_CPU_ONLY, hostBuffer, &hostBufferAllocation);
			}
			
			// Map data pointer to host buffer
			device->MapMemoryAllocation(hostBufferAllocation, (void**)&data);

			// Device buffer
			{VkBufferCreateInfo createInfo {};
				createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				createInfo.size = count * sizeof(T);
				createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				if constexpr (std::is_same_v<T, Mesh::VertexPosition>) {
					createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				}
				if constexpr (std::is_same_v<T, Mesh::Index16> || std::is_same_v<T, Mesh::Index32>) {
					createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				}
				if constexpr (std::is_same_v<T, Mesh::ProceduralVertexAABB>) {
					createInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
				}
				device->CreateAndAllocateBuffer(createInfo, MemoryUsage::MEMORY_USAGE_GPU_ONLY, deviceBuffer, &deviceBufferAllocation);
			}
			return data;
		}
		void Push(Device* device, VkCommandBuffer commandBuffer) {
			if (dirtyOnDevice) {
				assert(data && deviceBuffer && hostBuffer);
				VkBufferCopy region = {};{
					region.srcOffset = 0;
					region.dstOffset = 0;
					region.size = count * sizeof(T);
				}
				device->CmdCopyBuffer(commandBuffer, hostBuffer, deviceBuffer, 1, &region);
				dirtyOnDevice = false;
			}
		}
		void Pull(Device* device, VkCommandBuffer commandBuffer) {
			assert(deviceBuffer && hostBuffer);
			VkBufferCopy region = {};{
				region.srcOffset = 0;
				region.dstOffset = 0;
				region.size = count * sizeof(T);
			}
			device->CmdCopyBuffer(commandBuffer, deviceBuffer, hostBuffer, 1, &region);
		}
		void FreeBuffers(Device* device) {
			if (data) {
				assert(hostBuffer && hostBufferAllocation && deviceBuffer && deviceBufferAllocation); 
				device->UnmapMemoryAllocation(hostBufferAllocation);
				data = nullptr;
				device->FreeAndDestroyBuffer(hostBuffer, hostBufferAllocation);
				device->FreeAndDestroyBuffer(deviceBuffer, deviceBufferAllocation);
			}
			dirtyOnDevice = false;
		}
		T* operator->() {
			assert(data);
			return data;
		}
		template<typename _T>
		void Set(uint32_t index, _T&& value) {
			assert(data && index < count);
			data[index] = std::forward<_T>(value);
		}
		template<typename _T>
		void Set(_T&& value) {
			assert(data && count > 0);
			*data = std::forward<_T>(value);
		}
		T& Get(uint32_t index) {
			assert(data && index < count);
			return data[index];
		}
	};

	namespace Material {
		
	}
	
}
