#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB ShaderPipeline : public ShaderProgram {
	public:
		using ShaderProgram::ShaderProgram;
		virtual ~ShaderPipeline();
		
		virtual void Execute(Device* device, VkCommandBuffer cmdBuffer, uint32_t instanceCount, void* pushConstant, int pushConstantIndex = 0);
		virtual void Execute(Device* device, VkCommandBuffer cmdBuffer);
		
		virtual void CreatePipeline(Device*) = 0;
		virtual void DestroyPipeline(Device*) = 0;
		
		void PushConstant(Device* device, VkCommandBuffer cmdBuffer, void* pushConstant, int pushConstantIndex = 0);
		
	protected:
		VkPipeline pipeline = VK_NULL_HANDLE;
		
		// pure virtual methods
		virtual void Bind(Device*, VkCommandBuffer) = 0;
		virtual void Render(Device*, VkCommandBuffer, uint32_t instanceCount) = 0;
	};
	
}
