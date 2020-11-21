#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYGLTFLOADER

	namespace v4d::scene {

		struct GltfModelData {
			std::string_view filePath;
			std::vector<Geometry::VertexBuffer_T> preloadedVertices {};
			std::vector<uint32_t> preloadedIndices {};
			std::unordered_map<Geometry::VertexBuffer_T, uint32_t> preloadedUniqueVertices {};
			std::weak_ptr<Geometry> modelGeometry;
		};

		class V4DLIB GltfModelLoader : public ModelLoader<GltfModelData> {
		public:
			GltfModelLoader(std::string_view filePath);
			void Load() override;
			void Generate(ObjectInstance* obj) override;
		};
	}

#endif
