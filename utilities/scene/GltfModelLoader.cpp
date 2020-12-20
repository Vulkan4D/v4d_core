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
				
				if (mesh.name == "collider") {
					// The Collider mesh
					for (auto& p : mesh.primitives) {
						ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
						auto* geometryData = &modelData->colliderGeometry;
						
						// Indices
						auto& primitiveIndices = model.accessors[p.indices];
						auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
						geometryData->indexCount = primitiveIndices.count;
						switch (primitiveIndices.componentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
								geometryData->index16Buffer = reinterpret_cast<Index16*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							break;}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
								geometryData->index32Buffer = reinterpret_cast<Index32*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							break;}
							default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
						}
						ASSERT_OR_RETURN_FALSE(geometryData->indexCount > 0);
						ASSERT_OR_RETURN_FALSE(geometryData->index16Buffer || geometryData->index32Buffer);
						
						// Vertex data
						for (auto&[name,accessorIndex] : p.attributes) {
							if (name == "POSITION") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
								geometryData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 3);
								geometryData->vertexPositionBuffer = reinterpret_cast<VertexPosition*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							}
						}
						ASSERT_OR_RETURN_FALSE(geometryData->vertexCount > 0);
					}
				} else {
					// The Actual model high-poly geometry
					modelData->geometries[mesh.name].reserve(mesh.primitives.size());
					for (auto& p : mesh.primitives) {
						ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
						auto* geometryData = &modelData->geometries[mesh.name].emplace_back();
						
						// Material
						auto& material = model.materials[p.material];
						// material.pbrMetallicRoughness.baseColorFactor[0]
						// material.pbrMetallicRoughness.metallicFactor
						// material.pbrMetallicRoughness.roughnessFactor
						// geometryData->material = ....
						
						// Indices
						auto& primitiveIndices = model.accessors[p.indices];
						auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
						geometryData->indexCount = primitiveIndices.count;
						switch (primitiveIndices.componentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
								geometryData->index16Buffer = reinterpret_cast<Index16*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
								geometryData->indexStart = modelData->index16Count;
								modelData->index16Count += primitiveIndices.count;
							break;}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
								geometryData->index32Buffer = reinterpret_cast<Index32*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
								geometryData->indexStart = modelData->index32Count;
								modelData->index32Count += primitiveIndices.count;
							break;}
							default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
						}
						ASSERT_OR_RETURN_FALSE(geometryData->indexCount > 0);
						ASSERT_OR_RETURN_FALSE(geometryData->index16Buffer || geometryData->index32Buffer);
						
						// Vertex data
						for (auto&[name,accessorIndex] : p.attributes) {
							if (name == "POSITION") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
								geometryData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexPosition));
								geometryData->vertexPositionBuffer = reinterpret_cast<VertexPosition*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								geometryData->vertexPositionStart = modelData->vertexPositionCount;
								modelData->vertexPositionCount += vertices.count;
							} else if (name == "NORMAL") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
								geometryData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexNormal));
								geometryData->vertexNormalBuffer = reinterpret_cast<VertexNormal*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								geometryData->vertexNormalStart = modelData->vertexNormalCount;
								modelData->vertexNormalCount += vertices.count;
							} else if (name == "TEXCOORD_0") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
								geometryData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexUV));
								geometryData->vertexUVBuffer = reinterpret_cast<VertexUV*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								geometryData->vertexUVStart = modelData->vertexUVCount;
								modelData->vertexUVCount += vertices.count;
							} else if (name == "COLOR_0") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(geometryData->vertexCount == 0 || geometryData->vertexCount == vertices.count);
								geometryData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								switch (vertices.componentType) {
									case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : {
										ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorU8));
										geometryData->vertexColorU8Buffer = reinterpret_cast<VertexColorU8*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
										geometryData->vertexColorU8Start = modelData->vertexColorU8Count;
										modelData->vertexColorU8Count += vertices.count;
									break;}
									case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
										ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorU16));
										geometryData->vertexColorU16Buffer = reinterpret_cast<VertexColorU16*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
										geometryData->vertexColorU16Start = modelData->vertexColorU16Count;
										modelData->vertexColorU16Count += vertices.count;
									break;}
									case TINYGLTF_COMPONENT_TYPE_FLOAT: {
										ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorF32));
										geometryData->vertexColorF32Buffer = reinterpret_cast<VertexColorF32*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
										geometryData->vertexColorF32Start = modelData->vertexColorF32Count;
										modelData->vertexColorF32Count += vertices.count;
									break;}
									default: throw std::runtime_error("Vertex Color attributes only supports 8 or 16 bit unsigned integer or 32-bit float components");
								}
								ASSERT_OR_RETURN_FALSE(geometryData->vertexColorU8Buffer || geometryData->vertexColorU16Buffer || geometryData->vertexColorF32Buffer);
							}
						}
						ASSERT_OR_RETURN_FALSE(geometryData->vertexCount > 0);
						
						modelData->geometriesCount++;
					}
				}
			}
			
			return true;
		}
		
		void GltfModelLoader::Generate(v4d::graphics::RenderableGeometryEntity* entity, v4d::graphics::vulkan::Device* device) {
			if (modelData->geometriesCount == 0) return;
			entity->Allocate(device, "default", modelData->geometriesCount);
			
			if (modelData->index16Count)
				entity->Add_meshIndices16()->AllocateBuffersCount(device, modelData->index16Count);
			if (modelData->index32Count)
				entity->Add_meshIndices32()->AllocateBuffersCount(device, modelData->index32Count);
			if (modelData->vertexPositionCount)
				entity->Add_meshVertexPosition()->AllocateBuffersCount(device, modelData->vertexPositionCount);
			if (modelData->vertexNormalCount)
				entity->Add_meshVertexNormal()->AllocateBuffersCount(device, modelData->vertexNormalCount);
			if (modelData->vertexColorU8Count)
				entity->Add_meshVertexColorU8()->AllocateBuffersCount(device, modelData->vertexColorU8Count);
			if (modelData->vertexColorU16Count)
				entity->Add_meshVertexColorU16()->AllocateBuffersCount(device, modelData->vertexColorU16Count);
			if (modelData->vertexColorF32Count)
				entity->Add_meshVertexColorF32()->AllocateBuffersCount(device, modelData->vertexColorF32Count);
			if (modelData->vertexUVCount)
				entity->Add_meshVertexUV()->AllocateBuffersCount(device, modelData->vertexUVCount);
			
			for (auto&[name, geometries] : modelData->geometries) {
				for (auto& geometry : geometries) {
					auto& geom = entity->geometries.emplace_back();
					geom.transform = geometry.transform;
					geom.material = geometry.material;
					geom.indexCount = geometry.indexCount;
					geom.vertexCount = geometry.vertexCount;
					
					if (geometry.index16Buffer && entity->meshIndices16) {
						memcpy(&entity->meshIndices16.Lock()->data[geometry.indexStart], geometry.index16Buffer, geometry.indexCount * sizeof(Index16));
						geom.firstIndex = geometry.indexStart;
					}
					if (geometry.index32Buffer && entity->meshIndices32) {
						memcpy(&entity->meshIndices32.Lock()->data[geometry.indexStart], geometry.index32Buffer, geometry.indexCount * sizeof(Index32));
						geom.firstIndex = geometry.indexStart;
					}
					if (geometry.vertexPositionBuffer && entity->meshVertexPosition) {
						memcpy(&entity->meshVertexPosition.Lock()->data[geometry.vertexPositionStart], geometry.vertexPositionBuffer, geometry.vertexCount * sizeof(VertexPosition));
						geom.firstVertexPosition = geometry.vertexPositionStart;
					}
					if (geometry.vertexNormalBuffer && entity->meshVertexNormal) {
						memcpy(&entity->meshVertexNormal.Lock()->data[geometry.vertexNormalStart], geometry.vertexNormalBuffer, geometry.vertexCount * sizeof(VertexNormal));
						geom.firstVertexNormal = geometry.vertexNormalStart;
					}
					if (geometry.vertexColorU8Buffer && entity->meshVertexColorU8) {
						memcpy(&entity->meshVertexColorU8.Lock()->data[geometry.vertexColorU8Start], geometry.vertexColorU8Buffer, geometry.vertexCount * sizeof(VertexColorU8));
						geom.firstVertexColorU8 = geometry.vertexColorU8Start;
					}
					if (geometry.vertexColorU16Buffer && entity->meshVertexColorU16) {
						memcpy(&entity->meshVertexColorU16.Lock()->data[geometry.vertexColorU16Start], geometry.vertexColorU16Buffer, geometry.vertexCount * sizeof(VertexColorU16));
						geom.firstVertexColorU16 = geometry.vertexColorU16Start;
					}
					if (geometry.vertexColorF32Buffer && entity->meshVertexColorF32) {
						memcpy(&entity->meshVertexColorF32.Lock()->data[geometry.vertexColorF32Start], geometry.vertexColorF32Buffer, geometry.vertexCount * sizeof(VertexColorF32));
						geom.firstVertexColorF32 = geometry.vertexColorF32Start;
					}
					if (geometry.vertexUVBuffer && entity->meshVertexUV) {
						memcpy(&entity->meshVertexUV.Lock()->data[geometry.vertexUVStart], geometry.vertexUVBuffer, geometry.vertexCount * sizeof(VertexUV));
						geom.firstVertexUV = geometry.vertexUVStart;
					}
				}
			}
			
			// collider
			if (modelData->colliderGeometry.vertexCount && modelData->colliderGeometry.indexCount) {
				auto physics = entity->Add_physics();
				physics->mass = 10;
				physics->rigidbodyType = PhysicsInfo::RigidBodyType::DYNAMIC;
				if (modelData->colliderGeometry.index16Buffer) {
					physics->SetMeshCollider(modelData->colliderGeometry.vertexPositionBuffer, modelData->colliderGeometry.vertexCount, modelData->colliderGeometry.index16Buffer, modelData->colliderGeometry.indexCount);
				} else if (modelData->colliderGeometry.index32Buffer) {
					physics->SetMeshCollider(modelData->colliderGeometry.vertexPositionBuffer, modelData->colliderGeometry.vertexCount, modelData->colliderGeometry.index32Buffer, modelData->colliderGeometry.indexCount);
				}
			}
			
		}
	}
	
#endif
