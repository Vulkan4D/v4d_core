#include <v4d.h>

using namespace v4d::graphics::vulkan;

ShaderPipeline::~ShaderPipeline() {
	
};

void ShaderPipeline::Execute(Device* device, VkCommandBuffer cmdBuffer) {
	Bind(device, cmdBuffer);
	Render(device, cmdBuffer);
}
