#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {
	
	class V4DLIB ComputeShaderPipeline : public ShaderPipeline {
		uint32_t groupCountX = 0, groupCountY = 0, groupCountZ = 0;
		
	public:
		ComputeShaderPipeline(PipelineLayout& pipelineLayout, ShaderInfo shaderInfo);
		virtual ~ComputeShaderPipeline();
		
		virtual void CreatePipeline(Device* device) override;
		virtual void DestroyPipeline(Device* device) override;
		
		void SetGroupCounts(uint32_t x, uint32_t y, uint32_t z);
		
	protected:
		virtual void Bind(Device* device, VkCommandBuffer cmdBuffer) override;
		virtual void Render(Device* device, VkCommandBuffer cmdBuffer) override;
	};
	
}
