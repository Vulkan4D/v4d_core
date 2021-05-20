#pragma once

// TinyGltfLoader
#define TINYGLTF_USE_CPP14
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_INCLUDE_JSON
#include "utilities/data/json.hpp"
#include "../../tinygltf/tiny_gltf.h"

#include <v4d.h>
#include "utilities/io/ConfigFile.h"
#include "utilities/graphics/Mesh.hpp"

namespace v4d::graphics {
class V4DLIB MeshFile;
using MeshFilePtr = std::shared_ptr<MeshFile>;
class V4DLIB MeshFile {
	std::string filePath;
	tinygltf::Model gltfModel;
	
	std::unordered_map<std::string, Mesh> meshes {};
	std::unordered_map<std::string, glm::dvec3> transforms {};
	
	bool Load();
public:
	MeshFile(const std::string& filePath);
	static MeshFilePtr GetInstance(const std::string& filePath);
	
	std::string GetFilePath() const {return filePath;}
	
	bool ContainsMesh(const std::string& key) const {
		return meshes.contains(key);
	}
	bool ContainsTransform(const std::string& key) const {
		return transforms.contains(key);
	}
	
	Mesh& GetMesh(const std::string& key) {
		return meshes.at(key);
	}
	const Mesh& GetMesh(const std::string& key) const {
		return meshes.at(key);
	}
	glm::dvec3 GetTransform(const std::string& key) const {
		return transforms.at(key);
	}
	void ForEachMesh(std::function<void(const std::string& nodeName, Mesh& mesh, const glm::dvec3& transform)> func) {
		for (auto&[nodeName, mesh] : meshes) {
			func(nodeName, mesh, transforms[nodeName]);
		}
	}
	
	uint32_t GetMesh_geometriesCount(const std::string& nodeName) const {
		try {return GetMesh(nodeName).geometriesCount;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_index16Count(const std::string& nodeName) const {
		try {return GetMesh(nodeName).index16Count;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_index32Count(const std::string& nodeName) const {
		try {return GetMesh(nodeName).index32Count;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_vertexPositionCount(const std::string& nodeName) const {
		try {return GetMesh(nodeName).vertexPositionCount;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_vertexNormalCount(const std::string& nodeName) const {
		try {return GetMesh(nodeName).vertexNormalCount;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_vertexColorCount(const std::string& nodeName) const {
		try {return GetMesh(nodeName).vertexColorCount;}
		catch (...) {return 0;}
	}
	uint32_t GetMesh_vertexUvCount(const std::string& nodeName) const {
		try {return GetMesh(nodeName).vertexUvCount;}
		catch (...) {return 0;}
	}
};
}
