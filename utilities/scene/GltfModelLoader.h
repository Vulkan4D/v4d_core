#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYGLTFLOADER
	#include "tinygltf/tiny_gltf.h"

	namespace v4d::scene {
		using namespace v4d::graphics::Mesh;
		
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
