#ifdef _ENABLE_TINYGLTF

#define TINYGLTF_IMPLEMENTATION
#include "MeshFile.h"

namespace v4d::graphics {

static glm::dmat4 GetLocalTransform(const tinygltf::Node& node) {
	glm::dvec3 translation {0};
	glm::dquat rotation {1,0,0,0};
	glm::dvec3 scale {1};
	if (node.matrix.size() == 16) {
		return glm::dmat4(
			node.matrix[0],
			node.matrix[1],
			node.matrix[2],
			node.matrix[3],
			node.matrix[4],
			node.matrix[5],
			node.matrix[6],
			node.matrix[7],
			node.matrix[8],
			node.matrix[9],
			node.matrix[10],
			node.matrix[11],
			node.matrix[12],
			node.matrix[13],
			node.matrix[14],
			node.matrix[15]
		);
	} else {
		if (node.translation.size() == 3) {
			translation = glm::dvec3(node.translation[0], node.translation[1], node.translation[2]);
		}
		if (node.rotation.size() == 4) {
			rotation = glm::dquat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
		}
		if (node.scale.size() == 3) {
			scale = glm::dvec3(node.scale[0], node.scale[1], node.scale[2]);
		}
		return glm::translate(glm::dmat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::dmat4(1), scale);
	}
}

static glm::dmat4 GetLocalTransform(const std::vector<tinygltf::Node>& nodes, int nodeIndex) {
	return GetLocalTransform(nodes[nodeIndex]);
}

static glm::dmat4 GetAbsoluteTransform(std::map<int/*child*/, int/*parent*/>& childParentMap, std::vector<tinygltf::Node>& nodes, int nodeIndex) {
	if (childParentMap.contains(nodeIndex)) {
		return GetAbsoluteTransform(childParentMap, nodes, childParentMap[nodeIndex]) * GetLocalTransform(nodes, nodeIndex);
	} else {
		return GetLocalTransform(nodes, nodeIndex);
	}
}

static void FillNodeHierarchy(std::unique_ptr<mesh::Node>& root, const tinygltf::Node& node, const std::vector<tinygltf::Node>& allNodes) {
	root = std::make_unique<mesh::Node>();
	root->transform = GetLocalTransform(node);
	for (auto& childIndex : node.children) {
		auto& childNode = allNodes[childIndex];
		FillNodeHierarchy(root->children[childNode.name], childNode, allNodes);
	}
}

static VkFilter GetVkFilterFromGltf(int filter) {
	switch (filter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST: return VK_FILTER_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR: return VK_FILTER_LINEAR;
	}
	return VK_FILTER_LINEAR;
}

static VkSamplerAddressMode GetVkSamplerAddressModeFromGltf(int wrap) {
	switch (wrap) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	}
	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

MeshFilePtr MeshFile::GetInstance(const std::string& filePath)
	STATIC_CLASS_INSTANCES_CPP(filePath, MeshFile, filePath)

MeshFile::MeshFile(const std::string& filePath) : filePath(filePath) {
	LOG("Loading glTF model " << filePath)
	using namespace tinygltf;
	TinyGLTF loader;
	std::string err;
	std::string warn;
	if (!loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filePath)) {
		throw std::runtime_error(err);
	}
	if (warn != "") LOG(warn)
	if (!Load()) {
		throw std::runtime_error("Failed to load glTF model");
	}
}

bool MeshFile::Load() {
	using namespace mesh;
	
	// Load textures
	for (auto& image : gltfModel.images) {
		textures.emplace_back(std::make_shared<TextureObject>(image.width, image.height, image.component, image.image.data(), image.image.size()));
	}
	
	// Load nodes hierarchy
	std::map<int/*child*/, int/*parent*/> childParentMap {};
	for (size_t i = 0; i < gltfModel.nodes.size(); ++i) {
		for (auto& child : gltfModel.nodes[i].children) {
			childParentMap[child] = i;
		}
	}
	for (size_t i = 0; i < gltfModel.nodes.size(); ++i) if (!childParentMap.contains(i)) {
		auto& childNode = gltfModel.nodes[i];
		FillNodeHierarchy(rootNode.children[childNode.name], childNode, gltfModel.nodes);
	}
	
	// Load meshes and transforms
	for (size_t i = 0; i < gltfModel.nodes.size(); ++i) {
		auto& node = gltfModel.nodes[i];
		transforms[node.name] = GetAbsoluteTransform(childParentMap, gltfModel.nodes, i);
		
		if (node.mesh != -1) {
			// LOG("Loading mesh node '" << node.name << "'")
			
			auto& meshData = meshes[node.name];
			auto& geometryPrimitives = meshData.geometries;
			
			geometryPrimitives.reserve(gltfModel.meshes[node.mesh].primitives.size());
			
			for (auto& p : gltfModel.meshes[node.mesh].primitives) {
				ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
				auto* geometry = &geometryPrimitives.emplace_back();
				
				{// Indices
					auto& primitiveIndices = gltfModel.accessors[p.indices];
					auto& indexBufferView = gltfModel.bufferViews[primitiveIndices.bufferView];
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
					geometry->indexCount = primitiveIndices.count;
					switch (primitiveIndices.componentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
							geometry->indexBufferPtr_u16 = reinterpret_cast<Index16*>(&gltfModel.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							geometry->firstIndex = meshData.index16Count;
							meshData.index16Count += primitiveIndices.count;
						break;}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
							geometry->indexBufferPtr_u32 = reinterpret_cast<Index32*>(&gltfModel.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							geometry->firstIndex = meshData.index32Count;
							meshData.index32Count += primitiveIndices.count;
						break;}
						default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
					}
					ASSERT_OR_RETURN_FALSE(geometry->indexCount > 0);
					ASSERT_OR_RETURN_FALSE(geometry->indexBufferPtr_u16 || geometry->indexBufferPtr_u32);
				}
				
				size_t vertexPositionCount = 0;
				size_t vertexNormalCount = 0;
				size_t vertexColorCount = 0;
				size_t vertexTexCoord0Count = 0;
				size_t vertexTexCoord1Count = 0;
				size_t vertexTangentCount = 0;
				
				{// Vertex data
					for (auto&[name,accessorIndex] : p.attributes) {
						if (name == "POSITION") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexPositionF32Vec3));
							geometry->vertexBufferPtr_f32vec3 = reinterpret_cast<VertexPositionF32Vec3*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							geometry->firstVertex = meshData.vertexPositionCount;
							meshData.vertexPositionCount += vertices.count;
							vertexPositionCount += vertices.count;
						}
						else if (name == "NORMAL") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexNormalF32Vec3));
							geometry->normalBufferPtr_f32vec3 = reinterpret_cast<VertexNormalF32Vec3*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							geometry->firstNormal = meshData.vertexNormalCount;
							meshData.vertexNormalCount += vertices.count;
							vertexNormalCount += vertices.count;
						}
						else if (name == "TANGENT") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexTangentF32Vec4));
							geometry->tangentBufferPtr_f32vec4 = reinterpret_cast<VertexTangentF32Vec4*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							geometry->firstTangent = meshData.vertexTangentCount;
							meshData.vertexTangentCount += vertices.count;
							vertexTangentCount += vertices.count;
						}
						else if (name == "TEXCOORD_0") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexUvF32Vec2));
							geometry->texCoord0BufferPtr_f32vec2 = reinterpret_cast<VertexUvF32Vec2*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							geometry->firstTexCoord0 = meshData.vertexTexCoord0Count;
							meshData.vertexTexCoord0Count += vertices.count;
							vertexTexCoord0Count += vertices.count;
						}
						else if (name == "TEXCOORD_1") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexUvF32Vec2));
							geometry->texCoord1BufferPtr_f32vec2 = reinterpret_cast<VertexUvF32Vec2*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							geometry->firstTexCoord1 = meshData.vertexTexCoord1Count;
							meshData.vertexTexCoord1Count += vertices.count;
							vertexTexCoord1Count += vertices.count;
						}
						else if (name == "COLOR_0") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(geometry->vertexCount == 0 || geometry->vertexCount == vertices.count);
							geometry->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							switch (vertices.componentType) {
								case TINYGLTF_COMPONENT_TYPE_FLOAT: {
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorF32Vec4));
									geometry->colorBufferPtr_f32vec4 = reinterpret_cast<VertexColorF32Vec4*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
									geometry->firstColor = meshData.vertexColorCount;
									meshData.vertexColorCount += vertices.count;
									vertexColorCount += vertices.count;
								break;}
								default: throw std::runtime_error("Vertex Color attributes only supports 32-bit float components");
							}
						}
					}
					ASSERT_OR_RETURN_FALSE(geometry->vertexCount > 0);
				}
				
				ASSERT_OR_RETURN_FALSE(vertexPositionCount > 0);
				ASSERT_OR_RETURN_FALSE(vertexNormalCount == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexColorCount == 0 || vertexColorCount == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexTexCoord0Count == 0 || vertexTexCoord0Count == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexTexCoord1Count == 0 || vertexTexCoord1Count == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexTangentCount == 0 || vertexTangentCount == vertexPositionCount);
				
				if (p.material != -1) {// Material
					tinygltf::Material material = gltfModel.materials[p.material];
					geometry->materialName = material.name;
					geometry->baseColor = glm::vec4(material.pbrMetallicRoughness.baseColorFactor[0], material.pbrMetallicRoughness.baseColorFactor[1], material.pbrMetallicRoughness.baseColorFactor[2], material.pbrMetallicRoughness.baseColorFactor[3]);
					geometry->metallic = float(material.pbrMetallicRoughness.metallicFactor);
					geometry->roughness = float(material.pbrMetallicRoughness.roughnessFactor);
					// Normal texture
					if (material.normalTexture.index != -1) {
						auto& texture = gltfModel.textures[material.normalTexture.index];
						auto& sampler = gltfModel.samplers[texture.sampler];
						geometry->normalTexture = std::make_shared<SamplerObject>(textures[texture.source], GetVkFilterFromGltf(sampler.magFilter), GetVkFilterFromGltf(sampler.minFilter), GetVkSamplerAddressModeFromGltf(sampler.wrapS), GetVkSamplerAddressModeFromGltf(sampler.wrapT), GetVkSamplerAddressModeFromGltf(sampler.wrapR));
						// uint8_t texCoordIndex = material.normalTexture.texCoord; // 0/1 for texCoord0/texCoord1
					}
					// Albedo texture
					if (material.pbrMetallicRoughness.baseColorTexture.index != -1) {
						auto& texture = gltfModel.textures[material.pbrMetallicRoughness.baseColorTexture.index];
						auto& sampler = gltfModel.samplers[texture.sampler];
						geometry->albedoTexture = std::make_shared<SamplerObject>(textures[texture.source], GetVkFilterFromGltf(sampler.magFilter), GetVkFilterFromGltf(sampler.minFilter), GetVkSamplerAddressModeFromGltf(sampler.wrapS), GetVkSamplerAddressModeFromGltf(sampler.wrapT), GetVkSamplerAddressModeFromGltf(sampler.wrapR));
						// uint8_t texCoordIndex = material.pbrMetallicRoughness.baseColorTexture.texCoord; // 0/1 for texCoord0/texCoord1
					}
					// PBR texture (When exported from Blender's glTF (glb) binary exporter, we get a 3-component texture R=AO, G=Roughness, B=Metallic)
					if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
						auto& texture = gltfModel.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
						auto& sampler = gltfModel.samplers[texture.sampler];
						geometry->pbrTexture = std::make_shared<SamplerObject>(textures[texture.source], GetVkFilterFromGltf(sampler.magFilter), GetVkFilterFromGltf(sampler.minFilter), GetVkSamplerAddressModeFromGltf(sampler.wrapS), GetVkSamplerAddressModeFromGltf(sampler.wrapT), GetVkSamplerAddressModeFromGltf(sampler.wrapR));
						// uint8_t texCoordIndex = material.pbrMetallicRoughness.pbrTexture.texCoord; // 0/1 for texCoord0/texCoord1
					}
					// material.alphaMode // OPAQUE | MASK | BLEND???
				}
				
				meshData.geometriesCount++;
			}
		}
	}
	
	return true;
}

}

#endif
