#include <v4d.h>

#ifdef V4D_INCLUDE_TINYOBJLOADER

	#define TINYOBJLOADER_IMPLEMENTATION
	#include "tinyobjloader/tiny_obj_loader.h"

	namespace v4d::scene {
		ObjModelLoader::ObjModelLoader(std::string_view filePath, std::string_view baseDir) {
			modelData->objFilePath = filePath;
			modelData->objFileBaseDir = baseDir;
		}
		void ObjModelLoader::Load() {
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string err, warn;
			
			// Load file
			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelData->objFilePath.data(), modelData->objFileBaseDir.data())) {
				throw std::runtime_error(err);
			}
			if (warn != "") LOG_WARN(warn);
			
			// Reset data
			modelData->preloadedUniqueVertices.clear();
			modelData->preloadedVertices.clear();
			modelData->preloadedIndices.clear();
			
			// Fill data
			for (size_t i = 0; i < shapes.size(); i++) {
				auto& shape = shapes[i];
				uint32_t materialId = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : 0;
				for (size_t j = 0; j < shape.mesh.indices.size(); j++) {
					auto& index = shape.mesh.indices[j];
					materialId = shape.mesh.material_ids[j/3];
					Geometry::VertexBuffer_T vertex = {};
					vertex.pos = {
						attrib.vertices[3 * index.vertex_index + 0]*-1,
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]*-1,
					};
					// vertex.uv = {
					// 	attrib.texcoords[2 * index.texcoord_index + 0],
					// 	1.0f - attrib.texcoords[2 * index.texcoord_index + 1], // flipped vertical component
					// };
					vertex.SetUV({
						materials[materialId].metallic,
						materials[materialId].roughness
					});
					auto color = materials[materialId].diffuse;
					vertex.SetColor({color[0], color[1], color[2], 1});
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0]*-1,
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]*-1,
					};
					if (modelData->preloadedUniqueVertices.count(vertex) == 0) {
						modelData->preloadedUniqueVertices[vertex] = modelData->preloadedVertices.size();
						modelData->preloadedVertices.push_back(vertex);
					}
					modelData->preloadedIndices.push_back(modelData->preloadedUniqueVertices[vertex]);
				}
			}
		}
		void ObjModelLoader::Generate(ObjectInstance* obj) {
			auto geom = modelData->modelGeometry.lock();
			if (geom) {
				obj->AddGeometry("basic", geom);
			} else {
				geom = obj->AddGeometry("basic", modelData->preloadedVertices.size(), modelData->preloadedIndices.size());
				for (size_t i = 0; i < modelData->preloadedVertices.size(); ++i) {
					geom->SetVertex(i, modelData->preloadedVertices[i]);
				}
				geom->SetIndices(modelData->preloadedIndices.data());
				modelData->modelGeometry = geom;
			}
		}
	}

#endif
