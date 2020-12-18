#pragma once

#include <v4d.h>

namespace v4d::scene {
	template<class ModelDataType>
	class ModelLoader {
	public:
		virtual void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) = 0;
		std::shared_ptr<ModelDataType> modelData;
		virtual bool Load() = 0;
		void operator()(v4d::graphics::RenderableGeometryEntity* entity, v4d::graphics::vulkan::Device* device) {
			Generate(entity, device);
		}
		ModelLoader(const ModelLoader& original) : modelData(original.modelData) {}
		ModelLoader(ModelLoader&& original) : modelData(original.modelData) {}
		ModelLoader() : modelData(std::make_shared<ModelDataType>()) {}
		virtual ~ModelLoader(){}
	};
}

namespace v4d::scene::Geometry {
	struct VertexData {
		v4d::graphics::Mesh::VertexPosition pos;
		v4d::graphics::Mesh::VertexNormal normal;
		v4d::graphics::Mesh::VertexColor color;
		VertexData(){}
		VertexData(v4d::graphics::Mesh::VertexPosition pos, v4d::graphics::Mesh::VertexNormal normal, v4d::graphics::Mesh::VertexColor color) : pos(pos), normal(normal), color(color) {}
		bool operator == (const VertexData& other) const {
			return pos == other.pos && normal == other.normal && color == other.color;
		}
	};
}

namespace std {
	template<> struct hash<v4d::graphics::Mesh::VertexPosition> {
		size_t operator()(v4d::graphics::Mesh::VertexPosition const &vertex) const {
			return ((hash<glm::f32>()(vertex.x) ^
					(hash<glm::f32>()(vertex.y) << 1)) >> 1) ^
					(hash<glm::f32>()(vertex.z) << 1);
		}
	};
	template<> struct hash<v4d::graphics::Mesh::VertexNormal> {
		size_t operator()(v4d::graphics::Mesh::VertexNormal const &vertex) const {
			return ((hash<glm::f32>()(vertex.x) ^
					(hash<glm::f32>()(vertex.y) << 1)) >> 1) ^
					(hash<glm::f32>()(vertex.z) << 1);
		}
	};
	template<> struct hash<v4d::graphics::Mesh::VertexColor> {
		size_t operator()(v4d::graphics::Mesh::VertexColor const &vertex) const {
			return ((hash<glm::f32>()(vertex.r) ^
					(hash<glm::f32>()(vertex.g) << 1)) >> 1) ^
					(hash<glm::f32>()(vertex.b) << 1);
		}
	};
	template<> struct hash<v4d::scene::Geometry::VertexData> {
		size_t operator()(v4d::scene::Geometry::VertexData const &vertex) const {
			return ((hash<v4d::graphics::Mesh::VertexPosition>()(vertex.pos) ^
					(hash<v4d::graphics::Mesh::VertexNormal>()(vertex.normal) << 1)) >> 1) ^
					(hash<v4d::graphics::Mesh::VertexColor>()(vertex.color) << 1);
		}
	};
}
