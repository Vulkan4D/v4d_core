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
		uint32_t groupCountX = 0, groupCountY = 0, groupCountZ = 0;
		
	public:
		using ShaderPipelineObject::ShaderPipelineObject;
		
		virtual void Create(Device* device) override;
		virtual void Destroy() override;
		
		void SetGroupCounts(uint32_t x, uint32_t y, uint32_t z) {
			groupCountX = x;
			groupCountY = y;
			groupCountZ = z;
		}
		
	protected:
		using ShaderPipelineObject::Bind;
		using ShaderPipelineObject::Render;
		// these two methods are called automatically by Execute() from the parent class
		virtual void Bind(uint32_t frameIndex, VkCommandBuffer cmdBuffer) override {
			assert(device);
			device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, obj);
			GetPipelineLayout()->Bind(frameIndex, cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
		}
		virtual void Render(uint32_t/*unused_arg frameIndex*/, VkCommandBuffer cmdBuffer, uint32_t /*unused_arg instanceCount*/ = 0) override {
			assert(device);
			device->CmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
		}
	};
	
}
