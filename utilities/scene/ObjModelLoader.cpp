#include <v4d.h>

#ifdef V4D_INCLUDE_TINYOBJLOADER

	#define TINYOBJLOADER_IMPLEMENTATION
	#include "tinyobjloader/tiny_obj_loader.h"

	namespace v4d::scene {
		ObjModelLoader::ObjModelLoader(std::string_view filePath, std::string_view baseDir) {
			modelData->objFilePath = filePath;
			modelData->objFileBaseDir = baseDir;
		}
		bool ObjModelLoader::Load() {
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string err, warn;
			
			{// Load file
				if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelData->objFilePath.data(), modelData->objFileBaseDir.data())) {
					LOG_ERROR(err);
					return false;
				}
				if (warn != "") LOG_WARN(warn);
			}
			
			{// Reset data
				modelData->preloadedUniqueVertices.clear();
				modelData->preloadedIndices.clear();
				modelData->preloadedVertexPositions.clear();
				modelData->preloadedVertexNormals.clear();
				modelData->preloadedVertexColors.clear();
			}
			
			// Fill data
			for (size_t i = 0; i < shapes.size(); i++) {
				auto& shape = shapes[i];
				uint32_t materialId = shape.mesh.material_ids.size() > 0 ? shape.mesh.material_ids[0] : 0;
				for (size_t j = 0; j < shape.mesh.indices.size(); j++) {
					auto& index = shape.mesh.indices[j];
					materialId = shape.mesh.material_ids[j/3];
					v4d::scene::Geometry::VertexData vertex = {};
					vertex.pos = {
						attrib.vertices[3 * index.vertex_index + 0]*-1,
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]*-1,
					};
					// vertex.uv = {
					// 	attrib.texcoords[2 * index.texcoord_index + 0],
					// 	1.0f - attrib.texcoords[2 * index.texcoord_index + 1], // flipped vertical component
					// };
					// vertex.SetUV({
					// 	materials[materialId].metallic,
					// 	materials[materialId].roughness
					// });
					auto color = materials[materialId].diffuse;
					vertex.color = glm::u8vec4(glm::vec4{color[0], color[1], color[2], 1} * 255.0f);
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0]*-1,
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]*-1,
					};
					if (modelData->preloadedUniqueVertices.count(vertex) == 0) {
						modelData->preloadedUniqueVertices[vertex] = modelData->preloadedVertexPositions.size();
						modelData->preloadedVertexPositions.push_back(vertex.pos);
						modelData->preloadedVertexNormals.push_back(vertex.normal);
						modelData->preloadedVertexColors.push_back(vertex.color);
					}
					modelData->preloadedIndices.push_back(modelData->preloadedUniqueVertices[vertex]);
				}
			}
			
			return true;
		}
		void ObjModelLoader::Generate(v4d::graphics::RenderableGeometryEntity* entity, v4d::graphics::vulkan::Device* device) {
			auto sharedGeometryData = modelData->commonGeometryData.lock();
			if (sharedGeometryData) {
				entity->sharedGeometryData = sharedGeometryData;
				entity->Allocate(device, "default", 0);
				return;
			}
			entity->sharedGeometryData = std::make_shared<v4d::graphics::RenderableGeometryEntity::SharedGeometryData>();
			modelData->commonGeometryData = entity->sharedGeometryData;
			
			entity->Allocate(device, "default");
			entity->Add_meshIndices32()->AllocateBuffers(device, modelData->preloadedIndices.data(), modelData->preloadedIndices.size());
			entity->Add_meshVertexPosition()->AllocateBuffers(device, modelData->preloadedVertexPositions.data(), modelData->preloadedVertexPositions.size());
			entity->Add_meshVertexNormal()->AllocateBuffers(device, modelData->preloadedVertexNormals.data(), modelData->preloadedVertexNormals.size());
			entity->Add_meshVertexColorU8()->AllocateBuffers(device, modelData->preloadedVertexColors.data(), modelData->preloadedVertexColors.size());
		}
	}

#endif
