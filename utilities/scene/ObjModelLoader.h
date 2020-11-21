#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYOBJLOADER

	namespace v4d::scene {

		struct ObjModelData {
			std::string_view objFilePath;
			std::string_view objFileBaseDir;
			std::vector<Geometry::VertexBuffer_T> preloadedVertices {};
			std::vector<uint32_t> preloadedIndices {};
			std::unordered_map<Geometry::VertexBuffer_T, uint32_t> preloadedUniqueVertices {};
			std::weak_ptr<Geometry> modelGeometry;
		};

		class V4DLIB ObjModelLoader : public ModelLoader<ObjModelData> {
		public:
			ObjModelLoader(std::string_view filePath, std::string_view baseDir);
			void Load() override;
			void Generate(ObjectInstance* obj) override;
		};
	}

#endif
