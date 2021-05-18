/*
 * Vulkan Compute pipeline abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class extends from ShaderPipelineObject with specific functionality for compute shaders
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/Shader.h"
#include "utilities/graphics/vulkan/ShaderPipelineObject.h"

namespace v4d::graphics::vulkan {
	
	class V4DLIB ComputeShaderPipelineObject : public ShaderPipelineObject {
	
	public:
		using ShaderPipelineObject::ShaderPipelineObject;
		
		virtual void Create(Device* device) override;
		virtual void Destroy() override;
		
		virtual void Bind(VkCommandBuffer cmdBuffer, uint32_t frameIndex = 0) override {
			assert(device);
			device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, obj);
			GetPipelineLayout()->Bind(frameIndex, cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
		}
		virtual void Dispatch(VkCommandBuffer cmdBuffer, uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1) {
			assert(device);
			device->CmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
		}
	};
	
}
