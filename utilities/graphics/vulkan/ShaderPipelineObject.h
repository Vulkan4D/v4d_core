/*
 * Vulkan shader-related Pipeline abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class handles the binding and draw calls related to a specific shader program
 */
#pragma once

#include <v4d.h>
#include <vector>
#include <string>
#include <optional>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/ShaderProgram.h"
#include "utilities/io/FilePath.h"
#include "utilities/io/StringListFile.h"

namespace v4d::graphics::vulkan {

	class ShaderPipelineObject;
	
	struct ShaderPipelineMetaFile {
		v4d::io::FilePath file;
		std::vector<v4d::graphics::vulkan::ShaderPipelineObject*> shaders;
		double mtime;
		
		ShaderPipelineMetaFile(v4d::io::FilePath metaFile, std::vector<v4d::graphics::vulkan::ShaderPipelineObject*>&& shaders)
		: file(metaFile), shaders(shaders), mtime(0) {}
		
		ShaderPipelineMetaFile(const std::string& shaderProgram, ShaderPipelineObject* shaderPipeline)
		: file(shaderProgram+".meta"), shaders({shaderPipeline}), mtime(0) {}
		
		operator const std::vector<ShaderInfo> () const {
			std::vector<ShaderInfo> vec {};
			std::string path = file.GetParentPath();
			v4d::io::StringListFile::Instance(file)->Load([&path,&vec](v4d::io::ASCIIFile* file){
				for (auto& shader : ((v4d::io::StringListFile*)file)->lines) {
					vec.emplace_back(path + "/" + shader);
				}
			});
			return vec;
		}
	};

	class V4DLIB ShaderPipelineObject {
		COMMON_OBJECT (ShaderPipelineObject, VkPipeline, V4DLIB)
		COMMON_OBJECT_MOVEABLE(ShaderPipelineObject)
		COMMON_OBJECT_COPYABLE(ShaderPipelineObject)
		
	protected:
		ShaderPipelineMetaFile shaderPipelineMetaFile;
		ShaderProgram shaderProgram;
		PipelineLayoutObject* pipelineLayout = nullptr;
	public:
		int sortIndex = 0;
		Device* device = nullptr;
	
		ShaderPipelineObject(PipelineLayoutObject* pipelineLayout, const char* shaderFile, int sortIndex = 0)
		: shaderPipelineMetaFile(shaderFile, this), shaderProgram(shaderPipelineMetaFile), pipelineLayout(pipelineLayout), sortIndex(sortIndex) {}
		
		virtual ~ShaderPipelineObject() = default;
		
		operator ShaderPipelineMetaFile() {
			return shaderPipelineMetaFile;
		}
		
		inline void ReadShaders() {
			shaderProgram.ReadShaders();
		}
		
		virtual void Create(Device*) = 0;
		virtual void Destroy() = 0;
		virtual void Reload() {
			assert(device);
			Destroy();
			shaderProgram.ReadShaders();
			Create(device);
		}
		
		template<typename T>
		void SetConstantValue(const std::string& stage, uint32_t id, const T& value) {
			shaderProgram.SetConstantValue(stage, id, value);
		}
		
		void SetPipelineLayout(PipelineLayoutObject* layout) {
			this->pipelineLayout = layout;
		}

		PipelineLayoutObject* GetPipelineLayout() const {
			return pipelineLayout;
		}

		// sends a push constant to the shader now
		void PushConstant(VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex = 0);
		
		static uint CompactIVec4ToUint(uint r, uint g, uint b, uint a);
		static uint CompactVec4ToUint(float r, float g, float b, float a);
		static float CompactVec3rgb10ToFloat(float r, float g, float b);
		
		std::string GetShaderPath(std::string type) const;
		
		virtual void Bind(VkCommandBuffer, uint32_t frameIndex = 0) = 0;
	};
	
}
