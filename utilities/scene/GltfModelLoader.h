#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/Mesh.hpp"
#include "utilities/scene/PhysicsInfo.h"
#include "utilities/graphics/RenderableGeometryEntity.h"
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/scene/ModelLoader.hpp"

#ifdef V4D_INCLUDE_TINYGLTFLOADER

	#define TINYGLTF_USE_CPP14
	#define TINYGLTF_NO_INCLUDE_STB_IMAGE
	#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
	#define TINYGLTF_NO_STB_IMAGE
	#define TINYGLTF_NO_STB_IMAGE_WRITE
	#define TINYGLTF_NO_EXTERNAL_IMAGE
	#define TINYGLTF_NO_INCLUDE_JSON
	
	#include "json.hpp"
	#include "tinygltf/tiny_gltf.h"

	namespace v4d::scene {
		using namespace v4d::graphics::Mesh;
		
		// struct GeometryVertexAccessor {
		// 	union {
		// 		uint64_t packed;
		// 		struct {
		// 			uint32_t primitiveIndex;
		// 			uint32_t vertexIndex;
		// 		};
		// 	};
		// };
		
		// struct GeometryJoint {
		// 	std::string name;
		// 	glm::mat4 inverseBindMatrix;
		// 	glm::mat4 nodeMatrix;
		// 	std::vector<int> childJoints {};
		// 	std::map<uint64_t/*GeometryVertexAccessor*/, float/*weight*/> affectedVertices {};
		// };
		
		struct GltfGeometryData {
			glm::mat4 transform {1};
			std::string_view materialName {""};
			v4d::graphics::RenderableGeometryEntity::Material material {};
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			uint32_t indexStart = 0;
			Index16* index16Buffer = nullptr;
			Index32* index32Buffer = nullptr;
			VertexPosition* vertexPositionBuffer = nullptr;
			uint32_t vertexPositionStart = 0;
			VertexNormal* vertexNormalBuffer = nullptr;
			uint32_t vertexNormalStart = 0;
			VertexColorU8* vertexColorU8Buffer = nullptr;
			uint32_t vertexColorU8Start = 0;
			VertexColorU16* vertexColorU16Buffer = nullptr;
			uint32_t vertexColorU16Start = 0;
			VertexColorF32* vertexColorF32Buffer = nullptr;
			uint32_t vertexColorF32Start = 0;
			VertexUV* vertexUVBuffer = nullptr;
			uint32_t vertexUVStart = 0;
		};
		
		struct GltfColliderData {
			glm::mat4 transform {1};
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			Index16* index16Buffer = nullptr;
			Index32* index32Buffer = nullptr;
			VertexPosition* vertexPositionBuffer = nullptr;
		};

		struct GltfModelData {
			std::string_view filePath;
			tinygltf::Model gltfModel {};
			std::weak_ptr<v4d::graphics::RenderableGeometryEntity::SharedGeometryData> commonGeometryData;
			std::unordered_map<std::string, std::vector<GltfGeometryData>> geometries {};
			// std::unordered_map<std::string, std::unordered_map<uint32_t, GeometryJoint>> joints {};
			GltfColliderData colliderGeometry {};
			uint32_t geometriesCount = 0;
			
			uint32_t index16Count = 0;
			uint32_t index32Count = 0;
			uint32_t vertexPositionCount = 0;
			uint32_t vertexNormalCount = 0;
			uint32_t vertexColorU8Count = 0;
			uint32_t vertexColorU16Count = 0;
			uint32_t vertexColorF32Count = 0;
			uint32_t vertexUVCount = 0;
		};

		class V4DLIB GltfModelLoader : public ModelLoader<GltfModelData> {
			bool Load() override;
			void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) override;
		public:
			GltfModelLoader(std::string_view filePath);
		};
	}

#endif
