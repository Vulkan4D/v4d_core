#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYGLTFLOADER
	#include "tinygltf/tiny_gltf.h"

	namespace v4d::scene {
		
		struct GltfGeometryData {
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			uint16_t* indexBuffer16 = nullptr;
			uint32_t* indexBuffer32 = nullptr;
			float* vertexPositionBuffer = nullptr;
			float* vertexNormalBuffer = nullptr;
			float* vertexUVBuffer = nullptr;
			float* vertexColorBuffer = nullptr;
			uint16_t* vertexColorBufferUint16 = nullptr;
		};

		struct GltfModelData {
			std::string_view filePath;
			tinygltf::Model gltfModel;
			std::unordered_map<std::string, std::vector<GltfGeometryData>> geometries;
		};

		class V4DLIB GltfModelLoader : public ModelLoader<GltfModelData> {
		public:
			GltfModelLoader(std::string_view filePath);
			bool Load() override;
			void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) override;
		};
	}

#endif
