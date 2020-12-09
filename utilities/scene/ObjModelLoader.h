#pragma once

#include <v4d.h>

#ifdef V4D_INCLUDE_TINYOBJLOADER

	namespace v4d::scene::ObjModel {
		struct VertexData {
			Mesh::VertexPosition pos;
			Mesh::VertexNormal normal;
			Mesh::VertexColor color;
			VertexData(){}
			VertexData(Mesh::VertexPosition pos, Mesh::VertexNormal normal, Mesh::VertexColor color) : pos(pos), normal(normal), color(color) {}
			bool operator == (const VertexData& other) const {
				return pos == other.pos && normal == other.normal && color == other.color;
			}
		};
	}
	
	namespace std {
		template<> struct hash<v4d::scene::Mesh::VertexPosition> {
			size_t operator()(v4d::scene::Mesh::VertexPosition const &vertex) const {
				return ((hash<glm::f32>()(vertex.x) ^
						(hash<glm::f32>()(vertex.y) << 1)) >> 1) ^
						(hash<glm::f32>()(vertex.z) << 1);
			}
		};
		template<> struct hash<v4d::scene::Mesh::VertexNormal> {
			size_t operator()(v4d::scene::Mesh::VertexNormal const &vertex) const {
				return ((hash<glm::f32>()(vertex.x) ^
						(hash<glm::f32>()(vertex.y) << 1)) >> 1) ^
						(hash<glm::f32>()(vertex.z) << 1);
			}
		};
		template<> struct hash<v4d::scene::Mesh::VertexColor> {
			size_t operator()(v4d::scene::Mesh::VertexColor const &vertex) const {
				return ((hash<glm::f32>()(vertex.r) ^
						(hash<glm::f32>()(vertex.g) << 1)) >> 1) ^
						(hash<glm::f32>()(vertex.b) << 1);
			}
		};
		template<> struct hash<v4d::scene::ObjModel::VertexData> {
			size_t operator()(v4d::scene::ObjModel::VertexData const &vertex) const {
				return ((hash<v4d::scene::Mesh::VertexPosition>()(vertex.pos) ^
						(hash<v4d::scene::Mesh::VertexNormal>()(vertex.normal) << 1)) >> 1) ^
						(hash<v4d::scene::Mesh::VertexColor>()(vertex.color) << 1);
			}
		};
	}

	namespace v4d::scene {

		struct ObjModelData {
			std::string_view objFilePath;
			std::string_view objFileBaseDir;
			std::vector<v4d::scene::Mesh::VertexPosition> preloadedVertexPositions {};
			std::vector<v4d::scene::Mesh::VertexNormal> preloadedVertexNormals {};
			std::vector<v4d::scene::Mesh::VertexColor> preloadedVertexColors {};
			std::vector<uint32_t> preloadedIndices {};
			std::unordered_map<v4d::scene::ObjModel::VertexData, uint32_t> preloadedUniqueVertices {};
		};

		class V4DLIB ObjModelLoader : public ModelLoader<ObjModelData> {
		public:
			ObjModelLoader(std::string_view filePath, std::string_view baseDir);
			void Load() override;
			void Generate(RenderableGeometryEntity*) override {}
			void Generate(v4d::graphics::vulkan::Device* device, RenderableGeometryEntity*);
		};
	}

#endif
