#define TINYGLTF_IMPLEMENTATION
#include <v4d.h>

#ifdef V4D_INCLUDE_TINYGLTFLOADER

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
			
			// Mesh
			for (size_t meshIndex = 0; meshIndex < model.meshes.size(); ++meshIndex) {
				auto& mesh = model.meshes[meshIndex];
				
				if (mesh.name == "collider") {
					// The Collider mesh
					for (auto& p : mesh.primitives) {
						ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
						auto* primitiveData = &modelData->colliderGeometry;
						
						// Indices
						auto& primitiveIndices = model.accessors[p.indices];
						auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
						ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
						primitiveData->indexCount = primitiveIndices.count;
						switch (primitiveIndices.componentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
								primitiveData->index16Buffer = reinterpret_cast<Index16*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							break;}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
								ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
								primitiveData->index32Buffer = reinterpret_cast<Index32*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							break;}
							default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
						}
						ASSERT_OR_RETURN_FALSE(primitiveData->indexCount > 0);
						ASSERT_OR_RETURN_FALSE(primitiveData->index16Buffer || primitiveData->index32Buffer);
						
						// Vertex data
						for (auto&[name,accessorIndex] : p.attributes) {
							if (name == "POSITION") {
								auto& vertices = model.accessors[accessorIndex];
								ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
								primitiveData->vertexCount = vertices.count;
								auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 3);
								primitiveData->vertexPositionBuffer = reinterpret_cast<VertexPosition*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							}
						}
						ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount > 0);
					}
				} else {
					auto& geometryPrimitives = modelData->geometries[mesh.name];
					geometryPrimitives.reserve(mesh.primitives.size());
					
					// // Rigging and skins
					// for (auto& node : model.nodes) if (node.skin != -1 && node.mesh == meshIndex) {
					// 	auto& skin = model.skins[node.skin];
					// 	auto& inverseBindMatricesAccessor = model.accessors[skin.inverseBindMatrices];
					// 	assert(inverseBindMatricesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					// 	auto& inverseBindMatricesCount = inverseBindMatricesAccessor.count;
					// 	assert(inverseBindMatricesCount == skin.joints.size());
					// 	auto& inverseBindMatricesBufferView = model.bufferViews[inverseBindMatricesAccessor.bufferView];
					// 	assert(inverseBindMatricesBufferView.byteLength == inverseBindMatricesCount * sizeof(glm::mat4));
					// 	glm::mat4* inverseBindMatrices = reinterpret_cast<glm::mat4*>(&model.buffers[inverseBindMatricesBufferView.buffer].data.data()[inverseBindMatricesBufferView.byteOffset]);
					// 	for (auto& j : skin.joints) {
					// 		auto& jointNode = model.nodes[j];
					// 		auto& joint = modelData->joints[mesh.name][j];
					// 		joint.name = jointNode.name;
					// 		joint.inverseBindMatrix = inverseBindMatrices[j];
					// 		joint.nodeMatrix = glm::mat4(1);
					// 		if (jointNode.translation.size() == 3) joint.nodeMatrix = glm::translate(joint.nodeMatrix, glm::vec3(jointNode.translation[0], jointNode.translation[1], jointNode.translation[2]));
					// 		if (jointNode.rotation.size() == 4) joint.nodeMatrix *= glm::mat4_cast(glm::quat((float)jointNode.rotation[0], (float)jointNode.rotation[1], (float)jointNode.rotation[2], (float)jointNode.rotation[3]));
					// 		if (jointNode.scale.size() == 3) joint.nodeMatrix = glm::scale(joint.nodeMatrix, glm::vec3(jointNode.scale[0], jointNode.scale[1], jointNode.scale[2]));
					// 		joint.childJoints = jointNode.children;
					// 		joint.affectedVertices.clear();
					// 	}
					// 	break;// only one skin per mesh
					// }
					
					// The Actual model high-poly geometry
					for (auto& p : mesh.primitives) {
						ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
						uint32_t primitiveIndex = geometryPrimitives.size();
						auto* primitiveData = &geometryPrimitives.emplace_back();
						
						// // Joints
						// uint8_t* joints = nullptr;
						// float* weights = nullptr;
						
						{// Indices
							auto& primitiveIndices = model.accessors[p.indices];
							auto& indexBufferView = model.bufferViews[primitiveIndices.bufferView];
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
							primitiveData->indexCount = primitiveIndices.count;
							switch (primitiveIndices.componentType) {
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
									ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
									primitiveData->index16Buffer = reinterpret_cast<Index16*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
									primitiveData->indexStart = modelData->index16Count;
									modelData->index16Count += primitiveIndices.count;
								break;}
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
									ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
									primitiveData->index32Buffer = reinterpret_cast<Index32*>(&model.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
									primitiveData->indexStart = modelData->index32Count;
									modelData->index32Count += primitiveIndices.count;
								break;}
								default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
							}
							ASSERT_OR_RETURN_FALSE(primitiveData->indexCount > 0);
							ASSERT_OR_RETURN_FALSE(primitiveData->index16Buffer || primitiveData->index32Buffer);
						}
						
						{// Vertex data
							for (auto&[name,accessorIndex] : p.attributes) {
								if (name == "POSITION") {
									auto& vertices = model.accessors[accessorIndex];
									ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
									primitiveData->vertexCount = vertices.count;
									auto& vertexBufferView = model.bufferViews[vertices.bufferView];
									ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexPosition));
									primitiveData->vertexPositionBuffer = reinterpret_cast<VertexPosition*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
									primitiveData->vertexPositionStart = modelData->vertexPositionCount;
									modelData->vertexPositionCount += vertices.count;
								} else if (name == "NORMAL") {
									auto& vertices = model.accessors[accessorIndex];
									ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
									primitiveData->vertexCount = vertices.count;
									auto& vertexBufferView = model.bufferViews[vertices.bufferView];
									ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexNormal));
									primitiveData->vertexNormalBuffer = reinterpret_cast<VertexNormal*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
									primitiveData->vertexNormalStart = modelData->vertexNormalCount;
									modelData->vertexNormalCount += vertices.count;
								} else if (name == "TEXCOORD_0") {
									auto& vertices = model.accessors[accessorIndex];
									ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
									primitiveData->vertexCount = vertices.count;
									auto& vertexBufferView = model.bufferViews[vertices.bufferView];
									ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexUV));
									primitiveData->vertexUVBuffer = reinterpret_cast<VertexUV*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
									primitiveData->vertexUVStart = modelData->vertexUVCount;
									modelData->vertexUVCount += vertices.count;
								} else if (name == "COLOR_0") {
									auto& vertices = model.accessors[accessorIndex];
									ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
									primitiveData->vertexCount = vertices.count;
									auto& vertexBufferView = model.bufferViews[vertices.bufferView];
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
									switch (vertices.componentType) {
										case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : {
											ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorU8));
											primitiveData->vertexColorU8Buffer = reinterpret_cast<VertexColorU8*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
											primitiveData->vertexColorU8Start = modelData->vertexColorU8Count;
											modelData->vertexColorU8Count += vertices.count;
										break;}
										case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
											ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorU16));
											primitiveData->vertexColorU16Buffer = reinterpret_cast<VertexColorU16*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
											primitiveData->vertexColorU16Start = modelData->vertexColorU16Count;
											modelData->vertexColorU16Count += vertices.count;
										break;}
										case TINYGLTF_COMPONENT_TYPE_FLOAT: {
											ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorF32));
											primitiveData->vertexColorF32Buffer = reinterpret_cast<VertexColorF32*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
											primitiveData->vertexColorF32Start = modelData->vertexColorF32Count;
											modelData->vertexColorF32Count += vertices.count;
										break;}
										default: throw std::runtime_error("Vertex Color attributes only supports 8 or 16 bit unsigned integer or 32-bit float components");
									}
								}
								// else if (name == "JOINTS_0") {
								// 	auto& vertices = model.accessors[accessorIndex];
								// 	ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
								// 	primitiveData->vertexCount = vertices.count;
								// 	auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								// 	ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(uint8_t) * 4);
								// 	joints = reinterpret_cast<uint8_t*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								// } else if (name == "WEIGHTS_0") {
								// 	auto& vertices = model.accessors[accessorIndex];
								// 	ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
								// 	primitiveData->vertexCount = vertices.count;
								// 	auto& vertexBufferView = model.bufferViews[vertices.bufferView];
								// 	ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
								// 	ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(float) * 4);
								// 	weights = reinterpret_cast<float*>(&model.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
								// }
							}
							ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount > 0);
						}
						
						// {// Skins
						// 	for (size_t vertexIndex = 0; vertexIndex < primitiveData->vertexCount; ++vertexIndex) {
						// 		for (int i = 0; i < 4; ++i) {
						// 			float& weight = weights[vertexIndex*4+i];
						// 			if (weight > 0) {
						// 				GeometryJoint& joint = modelData->joints[mesh.name][joints[vertexIndex*4+i]];
						// 				GeometryVertexAccessor vertex;
						// 				vertex.primitiveIndex = primitiveIndex;
						// 				vertex.vertexIndex = vertexIndex;
						// 				joint.affectedVertices[vertex.packed] = weight;
										
						// 				// // TEST
						// 				// 	primitiveData->vertexPositionBuffer[vertexIndex] = glm::vec3(  );
						// 				// //
						// 			}
						// 		}
						// 	}
						// }
						
						if (p.material != -1) {// Material
							v4d::graphics::RenderableGeometryEntity::Material mat {};
							auto& material = model.materials[p.material];
							mat.visibility.metallic = (uint8_t)glm::clamp(material.pbrMetallicRoughness.metallicFactor * 255.0f, 0.0, 255.0);
							mat.visibility.roughness = (uint8_t)glm::clamp(material.pbrMetallicRoughness.roughnessFactor * 255.0f, 0.0, 255.0);
							if (material.emissiveFactor[0] > 0 || material.emissiveFactor[1] > 0 || material.emissiveFactor[2] > 0) {
								mat.visibility.baseColor.r = (uint8_t)glm::clamp(material.emissiveFactor[0] * 255.0, 0.0, 255.0);
								mat.visibility.baseColor.g = (uint8_t)glm::clamp(material.emissiveFactor[1] * 255.0, 0.0, 255.0);
								mat.visibility.baseColor.b = (uint8_t)glm::clamp(material.emissiveFactor[2] * 255.0, 0.0, 255.0);
								mat.visibility.emission = 10;
							} else {
								mat.visibility.baseColor.r = (uint8_t)glm::clamp(material.pbrMetallicRoughness.baseColorFactor[0] * 255.0, 0.0, 255.0);
								mat.visibility.baseColor.g = (uint8_t)glm::clamp(material.pbrMetallicRoughness.baseColorFactor[1] * 255.0, 0.0, 255.0);
								mat.visibility.baseColor.b = (uint8_t)glm::clamp(material.pbrMetallicRoughness.baseColorFactor[2] * 255.0, 0.0, 255.0);
								mat.visibility.emission = 0;
							}
							primitiveData->materialName = material.name;
							primitiveData->material = mat;
						}
						
						modelData->geometriesCount++;
					}
				}
			}
			
			return true;
		}
		
		void GltfModelLoader::Generate(v4d::graphics::RenderableGeometryEntity* entity, v4d::graphics::vulkan::Device* device) {
			auto sharedGeometryData = modelData->commonGeometryData.lock();
			if (sharedGeometryData) {
				entity->sharedGeometryData = sharedGeometryData;
				entity->Allocate(device, "default", 0);
			} else {
				entity->sharedGeometryData = std::make_shared<v4d::graphics::RenderableGeometryEntity::SharedGeometryData>();
				modelData->commonGeometryData = entity->sharedGeometryData;
			
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
				
				entity->sharedGeometryData->geometries.clear();
				entity->sharedGeometryData->geometries.reserve(modelData->geometries.size());
				
				for (auto&[name, geometries] : modelData->geometries) {
					for (auto& geometry : geometries) {
						auto& geom = entity->sharedGeometryData->geometries.emplace_back();
						geom.transform = geometry.transform;
						geom.material = geometry.material;
						geom.materialName = geometry.materialName;
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
			}
			
			// collider
			if (modelData->colliderGeometry.vertexCount && modelData->colliderGeometry.indexCount) {
				auto physics = entity->Add_physics();
				physics->mass = 10;
				physics->rigidbodyType = PhysicsInfo::RigidBodyType::STATIC;
				if (modelData->colliderGeometry.index16Buffer) {
					physics->SetMeshCollider(modelData->colliderGeometry.vertexPositionBuffer, modelData->colliderGeometry.vertexCount, modelData->colliderGeometry.index16Buffer, modelData->colliderGeometry.indexCount);
				} else if (modelData->colliderGeometry.index32Buffer) {
					physics->SetMeshCollider(modelData->colliderGeometry.vertexPositionBuffer, modelData->colliderGeometry.vertexCount, modelData->colliderGeometry.index32Buffer, modelData->colliderGeometry.indexCount);
				}
			}
			
			// Special nodes
			for (auto& node : modelData->gltfModel.nodes) {
				if (node.name == "Camera" && node.translation.size() == 3) {
					entity->cameraOffset = glm::translate(glm::dmat4(1), glm::dvec3(node.translation[0], node.translation[1], node.translation[2]));
				}
			}
			
		}
	}
	
#endif
