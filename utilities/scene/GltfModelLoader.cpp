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
		bool GltfModelLoader::Load() {
			using namespace tinygltf;
			
			TinyGLTF loader;
			std::string err;
			std::string warn;
			
			// Load file
			if (!loader.LoadBinaryFromFile(&modelData->gltfModel, &err, &warn, modelData->filePath.data())) {
				throw std::runtime_error(err);
			}
			if (warn != "") LOG_WARN(warn);

			auto& model = modelData->gltfModel;
			
			for (auto& mesh : model.meshes) {
				LOG_DEBUG("Loading mesh '" << mesh.name << "'")
				
				modelData->geometries[mesh.name].reserve(mesh.primitives.size());
				
				for (auto& p : mesh.primitives) {
					ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
					auto* geometryData = &modelData->geometries[mesh.name].emplace_back();
					
					LOG("Loading primitive with " << model.accessors[p.indices].count << " indices")
					
					// Material
					auto& material = model.materials[p.material];
					// material.pbrMetallicRoughness.baseColorFactor[0]
					// material.pbrMetallicRoughness.metallicFactor
					// material.pbrMetallicRoughness.roughnessFactor
					
					// Indices
					auto& primitiveIndices = model.accessors[p.indices];
					auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
					geometryData->indexCount = primitiveIndices.count;
					switch (primitiveIndices.componentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(uint16_t));
							geometryData->indexBuffer16 = reinterpret_cast<uint16_t*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
						break;}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(uint32_t));
							geometryData->indexBuffer32 = reinterpret_cast<uint32_t*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
						break;}
						// default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
					}
					ASSERT_OR_RETURN_FALSE(geometryData->indexCount > 0);
					ASSERT_OR_RETURN_FALSE(geometryData->indexBuffer16 || geometryData->indexBuffer32);
					
					// Vertex data
					for (auto&[name,accessorIndex] : p.attributes) {
						LOG(name)
						if (name == "POSITION") {
							auto& vertices = model.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
							geometryData->vertexCount = vertices.count;
							auto& vertexBufferView = model.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 3);
							geometryData->vertexPositionBuffer = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
						} else if (name == "NORMAL") {
							auto& vertices = model.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
							geometryData->vertexCount = vertices.count;
							auto& vertexBufferView = model.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 3);
							geometryData->vertexNormalBuffer = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
						} else if (name == "TEXCOORD_0") {
							auto& vertices = model.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
							geometryData->vertexCount = vertices.count;
							auto& vertexBufferView = model.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 2);
							geometryData->vertexUVBuffer = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
						} else if (name == "COLOR_0") {
							auto& vertices = model.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
							geometryData->vertexCount = vertices.count;
							auto& vertexBufferView = model.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							switch (vertices.componentType) {
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(uint16_t) * 4);
									geometryData->vertexColorBufferUint16 = reinterpret_cast<uint16_t*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								break;}
								case TINYGLTF_COMPONENT_TYPE_FLOAT: {
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 4);
									geometryData->vertexColorBuffer = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								break;}
								// default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
							}
							ASSERT_OR_RETURN_FALSE(geometryData->vertexColorBufferUint16 || geometryData->vertexColorBuffer);
						}
					}
					ASSERT_OR_RETURN_FALSE(geometryData->vertexCount > 0);
				}
			}
			
			return true;
		}
		
		void GltfModelLoader::Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) {
			
		}
	}
	
#endif
