#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYOBJLOADER

	namespace v4d::scene {

		struct ObjModelData {
			std::string_view objFilePath;
			std::string_view objFileBaseDir;
			std::vector<v4d::graphics::Mesh::VertexPosition> preloadedVertexPositions {};
			std::vector<v4d::graphics::Mesh::VertexNormal> preloadedVertexNormals {};
			std::vector<v4d::graphics::Mesh::VertexColor> preloadedVertexColors {};
			std::vector<uint32_t> preloadedIndices {};
			std::unordered_map<v4d::scene::Geometry::VertexData, uint32_t> preloadedUniqueVertices {};
		};

		class V4DLIB ObjModelLoader : public ModelLoader<ObjModelData> {
		public:
			ObjModelLoader(std::string_view filePath, std::string_view baseDir);
			bool Load() override;
			void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) override;
		};
	}

#endif
