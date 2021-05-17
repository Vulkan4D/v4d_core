#define TINYGLTF_IMPLEMENTATION
#include "MeshFile.h"

namespace v4d::graphics {

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
	
	for (auto node : gltfModel.nodes) {
		glm::dvec3& translation = transforms[node.name] = {0,0,0};
		if (node.translation.size() == 3) {
			translation.x = node.translation[0];
			translation.y = node.translation[1];
			translation.z = node.translation[2];
		}
		
		if (node.mesh != -1) {
			// LOG("Loading mesh node '" << node.name << "'")
			
			auto& meshData = meshes[node.name];
			auto& geometryPrimitives = meshData.geometries;
			
			geometryPrimitives.reserve(gltfModel.meshes[node.mesh].primitives.size());
			
			for (auto& p : gltfModel.meshes[node.mesh].primitives) {
				ASSERT_OR_RETURN_FALSE(p.mode == TINYGLTF_MODE_TRIANGLES);
				auto* primitiveData = &geometryPrimitives.emplace_back();
				
				{// Indices
					auto& primitiveIndices = gltfModel.accessors[p.indices];
					auto& indexBufferView = gltfModel.bufferViews[primitiveIndices.bufferView];
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteStride == 0); // Only supports tightly packed buffers
					ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength > 0);
					primitiveData->indexCount = primitiveIndices.count;
					switch (primitiveIndices.componentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index16));
							primitiveData->indexBufferPtr_u16 = reinterpret_cast<Index16*>(&gltfModel.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							primitiveData->firstIndex = meshData.index16Count;
							meshData.index16Count += primitiveIndices.count;
						break;}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
							ASSERT_OR_RETURN_FALSE(indexBufferView.byteLength == primitiveIndices.count * sizeof(Index32));
							primitiveData->indexBufferPtr_u32 = reinterpret_cast<Index32*>(&gltfModel.buffers[indexBufferView.buffer].data.data()[indexBufferView.byteOffset]);
							primitiveData->firstIndex = meshData.index32Count;
							meshData.index32Count += primitiveIndices.count;
						break;}
						default: throw std::runtime_error("Index buffer only supports 16 or 32 bits unsigned integer components");
					}
					ASSERT_OR_RETURN_FALSE(primitiveData->indexCount > 0);
					ASSERT_OR_RETURN_FALSE(primitiveData->indexBufferPtr_u16 || primitiveData->indexBufferPtr_u32);
				}
				
				size_t vertexPositionCount = 0;
				size_t vertexNormalCount = 0;
				size_t vertexColorCount = 0;
				size_t vertexUvCount = 0;
				
				{// Vertex data
					for (auto&[name,accessorIndex] : p.attributes) {
						if (name == "POSITION") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
							primitiveData->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexPositionF32Vec3));
							primitiveData->vertexBufferPtr_f32vec3 = reinterpret_cast<VertexPositionF32Vec3*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							primitiveData->firstVertex = meshData.vertexPositionCount;
							meshData.vertexPositionCount += vertices.count;
							vertexPositionCount += vertices.count;
						}
						else if (name == "NORMAL") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
							primitiveData->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexNormalF32Vec3));
							primitiveData->normalBufferPtr_f32vec3 = reinterpret_cast<VertexNormalF32Vec3*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							primitiveData->firstNormal = meshData.vertexNormalCount;
							meshData.vertexNormalCount += vertices.count;
							vertexNormalCount += vertices.count;
						}
						else if (name == "TEXCOORD_0") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
							primitiveData->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertices.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexUvF32Vec2));
							primitiveData->uvBufferPtr_f32vec2 = reinterpret_cast<VertexUvF32Vec2*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
							primitiveData->firstUv = meshData.vertexUvCount;
							meshData.vertexUvCount += vertices.count;
							vertexUvCount += vertices.count;
						}
						else if (name == "COLOR_0") {
							auto& vertices = gltfModel.accessors[accessorIndex];
							ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount == 0 || primitiveData->vertexCount == vertices.count);
							primitiveData->vertexCount = vertices.count;
							auto& vertexBufferView = gltfModel.bufferViews[vertices.bufferView];
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteStride == 0); // Only supports tightly packed buffers
							ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength > 0);
							switch (vertices.componentType) {
								case TINYGLTF_COMPONENT_TYPE_FLOAT: {
									ASSERT_OR_RETURN_FALSE(vertexBufferView.byteLength == vertices.count * sizeof(VertexColorF32Vec4));
									primitiveData->colorBufferPtr_f32vec4 = reinterpret_cast<VertexColorF32Vec4*>(&gltfModel.buffers[vertexBufferView.buffer].data.data()[vertexBufferView.byteOffset]);
									primitiveData->firstColor = meshData.vertexColorCount;
									meshData.vertexColorCount += vertices.count;
									vertexColorCount += vertices.count;
								break;}
								default: throw std::runtime_error("Vertex Color attributes only supports 32-bit float components");
							}
						}
					}
					ASSERT_OR_RETURN_FALSE(primitiveData->vertexCount > 0);
				}
				
				ASSERT_OR_RETURN_FALSE(vertexPositionCount > 0);
				ASSERT_OR_RETURN_FALSE(vertexNormalCount == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexColorCount == 0 || vertexColorCount == vertexPositionCount);
				ASSERT_OR_RETURN_FALSE(vertexUvCount == 0 || vertexUvCount == vertexPositionCount);
				
				if (p.material != -1) {// Material
					primitiveData->materialName = gltfModel.materials[p.material].name;
				}
				
				meshData.geometriesCount++;
			}
		}
	}
	return true;
}

}
