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
		// these two methods are called automatically by Execute() from the parent class
		virtual void Bind(VkCommandBuffer cmdBuffer) override {
			assert(device);
			device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, obj);
			device->CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, GetPipelineLayout()->obj, 0, GetPipelineLayout()->vkDescriptorSets.size(), GetPipelineLayout()->vkDescriptorSets.data(), 0, nullptr);
		}
		virtual void Render(VkCommandBuffer cmdBuffer, uint32_t /*unused_arg*/ = 0) {
			assert(device);
			device->CmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
		}
	};
	
}
