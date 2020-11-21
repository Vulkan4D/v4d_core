#include <v4d.h>

// WORK IN PROGRESS....

#ifdef V4D_INCLUDE_TINYGLTFLOADER

	#define TINYGLTF_USE_CPP14
	#define TINYGLTF_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_WRITE_IMPLEMENTATION
	#define TINYGLTF_NO_EXTERNAL_IMAGE
	#define TINYGLTF_NO_INCLUDE_JSON
	#include "json.hpp"
	#include "tinygltf/tiny_gltf.h"

	namespace v4d::scene {
		GltfModelLoader::GltfModelLoader(std::string_view filePath) {
			modelData->filePath = filePath;
		}
		void GltfModelLoader::Load() {
			using namespace tinygltf;
			
			Model model;
			TinyGLTF loader;
			std::string err;
			std::string warn;
			
			// Load file
			if (!loader.LoadBinaryFromFile(&model, &err, &warn, modelData->filePath.data())) {
				throw std::runtime_error(err);
			}
			if (warn != "") LOG_WARN(warn);
			
			// Reset data
			modelData->preloadedUniqueVertices.clear();
			modelData->preloadedVertices.clear();
			modelData->preloadedIndices.clear();
			
			for (auto& mesh : model.meshes) {
				for (auto& p : mesh.primitives) {
					auto& material = model.materials[p.material];
					auto& primitiveIndices = model.accessors[p.indices];
					auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
					assert(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
					assert(indexBufferView.byteLength > 0);
					std::vector<Geometry::IndexBuffer_T> indices {};
					indices.reserve(primitiveIndices.count);
					switch (primitiveIndices.componentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
							assert(indexBufferView.byteLength == primitiveIndices.count * sizeof(uint16_t));
							uint16_t* indexBuffer = reinterpret_cast<uint16_t*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							for (size_t i = 0; i < primitiveIndices.count; ++i) indices.push_back(indexBuffer[i]);
						break;}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
							assert(indexBufferView.byteLength == primitiveIndices.count * sizeof(uint32_t));
							uint32_t* indexBuffer = reinterpret_cast<uint32_t*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							for (size_t i = 0; i < primitiveIndices.count; ++i) indices.push_back(indexBuffer[i]);
						break;}
						default: throw std::runtime_error("Index buffer only supports unsigned short/int components");
					}
					auto& primitiveVertices = model.accessors[p.attributes["POSITION"]];
					auto& primitiveNormals = model.accessors[p.attributes["NORMAL"]];
					auto& vertexBufferView = model.bufferViews[primitiveVertices.bufferView];
					auto& normalBufferView = model.bufferViews[primitiveNormals.bufferView];
					assert(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
					assert(normalBufferView.byteStride == 0); // Only supports tightly packed buffers
					assert(primitiveVertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					assert(primitiveNormals.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					assert(vertexBufferView.byteLength > 0);
					assert(normalBufferView.byteLength > 0);
					assert(vertexBufferView.byteLength == primitiveVertices.count * sizeof(float) * 3);
					assert(normalBufferView.byteLength == primitiveNormals.count * sizeof(float) * 3);
					float* vertexBuffer = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
					float* normalBuffer = reinterpret_cast<float*>(&model.buffers[normalBufferView.buffer].data.data()[normalBufferView.byteOffset]);
					for (auto& i : indices) {
						Geometry::VertexBuffer_T vertex {};{
							vertex.pos = glm::vec3(vertexBuffer[i*3], vertexBuffer[i*3+1], vertexBuffer[i*3+2]);
							vertex.normal = glm::vec3(normalBuffer[i*3], normalBuffer[i*3+1], normalBuffer[i*3+2]);
							vertex.SetColor(glm::vec4(material.pbrMetallicRoughness.baseColorFactor[0], material.pbrMetallicRoughness.baseColorFactor[1], material.pbrMetallicRoughness.baseColorFactor[2], material.pbrMetallicRoughness.baseColorFactor[3]));
							vertex.SetUV(glm::vec2(material.pbrMetallicRoughness.metallicFactor, material.pbrMetallicRoughness.roughnessFactor));
						}
						if (modelData->preloadedUniqueVertices.count(vertex) == 0) {
							modelData->preloadedUniqueVertices[vertex] = modelData->preloadedVertices.size();
							modelData->preloadedVertices.push_back(vertex);
						}
						modelData->preloadedIndices.push_back(modelData->preloadedUniqueVertices[vertex]);
					}
				}
			}
		}
		void GltfModelLoader::Generate(ObjectInstance* obj) {
			auto geom = modelData->modelGeometry.lock();
			if (geom) {
				obj->AddGeometry(geom);
			} else {
				geom = obj->AddGeometry("basic", modelData->preloadedVertices.size(), modelData->preloadedIndices.size());
				for (int i = 0; i < modelData->preloadedVertices.size(); ++i) {
					geom->SetVertex(i, modelData->preloadedVertices[i]);
				}
				geom->SetIndices(modelData->preloadedIndices.data());
				geom->colliderType = Geometry::ColliderType::SPHERE;
				geom->boundingDistance = 1;
				modelData->modelGeometry = geom;
			}
		}
	}
	
#endif
