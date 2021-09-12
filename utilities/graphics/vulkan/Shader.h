/*
 * Vulkan Shader file abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class is an abstraction of individual shader files within a shader program
 */
#pragma once

#include <v4d.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/io/FilePath.h"
#include "utilities/io/StringListFile.h"

namespace v4d::graphics::vulkan {

	struct V4DLIB ShaderInfo {
		std::string filepath;
		int subpass;
		std::string entryPoint;
		
		ShaderInfo(const std::string& filepath, int subpass, const std::string& entryPoint);
		ShaderInfo(const std::string& filepath, int subpass = 0);
		ShaderInfo(const char* filepath, int subpass = 0);
	};
	
	class ShaderPipelineObject;
	namespace raytracing {
		class ShaderBindingTable;
	};
	using namespace raytracing;
	
	struct ShaderPipelineMetaFile {
		v4d::io::FilePath file;
		std::vector<ShaderPipelineObject*> shaders;
		ShaderBindingTable* sbt;
		double mtime;
		
		ShaderPipelineMetaFile(v4d::io::FilePath metaFile, std::vector<ShaderPipelineObject*>&& shaders)
		: file(metaFile), shaders(shaders), sbt(nullptr), mtime(0) {}
		ShaderPipelineMetaFile(const std::string& shaderProgram, ShaderPipelineObject* shaderPipeline)
		: file(shaderProgram+".meta"), shaders({shaderPipeline}), sbt(nullptr), mtime(0) {}
		ShaderPipelineMetaFile(const std::string& shaderProgram, ShaderBindingTable* sbt)
		: file(shaderProgram+".meta"), shaders(), sbt(sbt), mtime(0) {}
		
		operator const std::vector<ShaderInfo> () const {
			DEBUG_ASSERT_ERROR(file.Exists(), "Shader file does not exist: " << std::string(file));
			if (!file.Exists()) return {};
			std::vector<ShaderInfo> vec {};
			std::string path = file.GetParentPath();
			v4d::io::StringListFile::Instance(file)->Load([&path,&vec](v4d::io::ASCIIFile* file){
				for (auto& lineStr : ((v4d::io::StringListFile*)file)->lines) {
					std::stringstream line {lineStr};
					int subpass;
					std::string shader;
					std::string entryPoint;
					line >> subpass;
					line >> shader;
					line >> entryPoint;
					if (entryPoint == "") entryPoint = "main";
					vec.emplace_back(path + "/" + shader, subpass, entryPoint);
				}
			});
			return vec;
		}
	};

	struct ShaderSpecialization {
		std::vector<byte> specializationData {};
		std::vector<VkSpecializationMapEntry> specializationMap {};
		VkSpecializationInfo specializationInfo {};
		
		template<typename T>
		void SetValue(uint32_t id, const T& value) {
			size_t offset;
			if (auto it = std::find_if(specializationMap.begin(), specializationMap.end(), [id](auto&s){
				return s.constantID == id;
			}); it != specializationMap.end()) {
				assert(it->size == sizeof(T));
				offset = it->offset;
			} else {
				offset = specializationData.size();
				specializationData.resize(offset + sizeof(T));
				specializationMap.emplace_back(id, offset, sizeof(T));
			}
			memcpy(specializationData.data() + offset, &value, sizeof(T));
		}
	};

	static std::unordered_map<std::string, VkShaderStageFlagBits> SHADER_TYPES {
		{"vert", VK_SHADER_STAGE_VERTEX_BIT},
		{"tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
		{"tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
		{"geom", VK_SHADER_STAGE_GEOMETRY_BIT},
		{"frag", VK_SHADER_STAGE_FRAGMENT_BIT},
		{"comp", VK_SHADER_STAGE_COMPUTE_BIT},
		{"mesh", VK_SHADER_STAGE_MESH_BIT_NV},
		{"task", VK_SHADER_STAGE_TASK_BIT_NV},
		{"rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
		{"rint", VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
		{"rahit", VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
		{"rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
		{"rmiss", VK_SHADER_STAGE_MISS_BIT_KHR},
		{"rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR},
	};

	class V4DLIB Shader {
	private:

		std::string filepath; // path relative to executable
		std::string entryPoint; // usually "main"
		
		std::vector<char> bytecode; // contains the spv file contents
		
		VkShaderModule module = VK_NULL_HANDLE;
		
	public:
		std::string name; // the shader file name without directory nor extension
		std::string type; // string key in SHADER_TYPES
		
		VkPipelineShaderStageCreateInfo stageInfo;

		ShaderSpecialization specialization {};
		
		Shader(std::string filepath/*relative to executable*/, std::string entryPoint = "main");
		
		VkShaderModule CreateShaderModule(Device* device, VkPipelineShaderStageCreateFlags flags = 0);
		void DestroyShaderModule(Device* device);

	};
}
