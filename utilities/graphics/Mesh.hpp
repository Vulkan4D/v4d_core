#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/ShaderProgram.h"

namespace v4d::graphics {
	namespace mesh {
		
		typedef uint16_t Index16;
		typedef uint32_t Index32;
		
		struct VertexPositionF32Vec3 {
			glm::f32 x;
			glm::f32 y;
			glm::f32 z;
			VertexPositionF32Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
			VertexPositionF32Vec3(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
			operator glm::vec3() {return glm::vec3(x, y, z);}
			VertexPositionF32Vec3() {static_assert(sizeof(VertexPositionF32Vec3) == 12);}
			static std::vector<v4d::graphics::vulkan::VertexInputAttributeDescription> GetInputAttributes() {
				return {
					{0, offsetof(VertexPositionF32Vec3, x), VK_FORMAT_R32G32B32_SFLOAT},
				};
			}
			bool operator==(const VertexPositionF32Vec3& other) const {
				return x == other.x && y == other.y && z == other.z;
			}
			bool operator!=(const VertexPositionF32Vec3& other) const {return !(*this == other);}
		};
		
		template<typename T>
		struct VertexNormalVec3 {
			T x;
			T y;
			T z;
			VertexNormalVec3(T x, T y, T z) : x(x), y(y), z(z) {}
			VertexNormalVec3(const glm::vec<3,T>& v) : x(v.x), y(v.y), z(v.z) {}
			operator glm::vec<3,T>() {return glm::vec<3,T>(x, y, z);}
			VertexNormalVec3() {static_assert(sizeof(VertexNormalVec3<T>) == sizeof(T)*3);}
			bool operator==(const VertexNormalVec3<T>& other) const {
				return x == other.x && y == other.y && z == other.z;
			}
			bool operator!=(const VertexNormalVec3<T>& other) const {return !(*this == other);}
		};
		using VertexNormalF32Vec3 = VertexNormalVec3<glm::f32>;
		
		template<typename T>
		struct VertexColorVec4 {
			T r;
			T g;
			T b;
			T a;
			VertexColorVec4() : r(0), g(0), b(0), a(0) {}
			VertexColorVec4(T r, T g, T b, T a) : r(r), g(g), b(b), a(a) {}
			VertexColorVec4(const glm::vec<4, T>& v) : r(v.r), g(v.g), b(v.b), a(v.a) {}
			operator glm::vec<4, T>() {return glm::vec<4, T>(r, g, b, a);}
			bool operator==(const VertexColorVec4& other) const {
				return r == other.r && g == other.g && b == other.b && a == other.a;
			}
			bool operator!=(const VertexColorVec4& other) const {return !(*this == other);}
		};
		using VertexColorF32Vec4 = VertexColorVec4<glm::f32>;
		
		template<typename T>
		struct VertexUvVec2 {
			T s;
			T t;
			VertexUvVec2(float s, float t) : s(s), t(t) {}
			VertexUvVec2(const glm::vec<2,T>& v) : s(v.s), t(v.t) {}
			operator glm::vec<2,T>() {return glm::vec<2,T>(s, t);}
			VertexUvVec2() {static_assert(sizeof(VertexUvVec2) == sizeof(T)*2);}
			bool operator==(const VertexUvVec2& other) const {
				return s == other.s && t == other.t;
			}
			bool operator!=(const VertexUvVec2& other) const {return !(*this == other);}
		};
		using VertexUvF32Vec2 = VertexUvVec2<glm::f32>;
		
		struct ProceduralVertexAABB {
			glm::vec3 aabbMin;
			glm::vec3 aabbMax;
			ProceduralVertexAABB(glm::vec3 aabbMin, glm::vec3 aabbMax) : aabbMin(aabbMin), aabbMax(aabbMax) {}
			ProceduralVertexAABB() {static_assert(sizeof(ProceduralVertexAABB) == 24);}
			static std::vector<v4d::graphics::vulkan::VertexInputAttributeDescription> GetInputAttributes() {
				return {
					{0, offsetof(ProceduralVertexAABB, aabbMin), VK_FORMAT_R32G32B32_SFLOAT},
					{1, offsetof(ProceduralVertexAABB, aabbMax), VK_FORMAT_R32G32B32_SFLOAT},
				};
			}
		};

		struct Geometry {
			std::string materialName {""};
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			uint32_t firstIndex = 0;
			Index16* indexBufferPtr_u16 = nullptr;
			Index32* indexBufferPtr_u32 = nullptr;
			VertexPositionF32Vec3* vertexBufferPtr_f32vec3 = nullptr;
			uint32_t firstVertex = 0;
			VertexNormalF32Vec3* normalBufferPtr_f32vec3 = nullptr;
			uint32_t firstNormal = 0;
			VertexColorF32Vec4* colorBufferPtr_f32vec4 = nullptr;
			uint32_t firstColor = 0;
			VertexUvF32Vec2* uvBufferPtr_f32vec2 = nullptr;
			uint32_t firstUv = 0;
		};
	}
	
	struct Mesh {
		std::vector<mesh::Geometry> geometries {};
		uint32_t geometriesCount = 0;
		uint32_t index16Count = 0;
		uint32_t index32Count = 0;
		uint32_t vertexPositionCount = 0;
		uint32_t vertexNormalCount = 0;
		uint32_t vertexColorCount = 0;
		uint32_t vertexUvCount = 0;
	};
}
