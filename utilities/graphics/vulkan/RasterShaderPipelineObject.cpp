#include "RasterShaderPipelineObject.h"
#include "utilities/graphics/vulkan/Instance.h"

using namespace v4d::graphics::vulkan;

void RasterShaderPipelineObject::Create(Device* device) {
	assert(this->device == nullptr);
	this->device = device;
	
	shaderProgram.CreateShaderStages(device);
	
	if (pipelineCreateInfo.pViewportState == nullptr) {
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = viewports.size();
		viewportState.scissorCount = scissors.size();
		viewportState.pViewports = viewports.size()>0? viewports.data() : nullptr;
		viewportState.pScissors = scissors.size()>0? scissors.data() : nullptr;
		pipelineCreateInfo.pViewportState = &viewportState;
	}
	
	// Bindings and Attributes
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// Dynamic states
	if (dynamicStates.size() > 0) {
		dynamicStateCreateInfo.dynamicStateCount = (uint)dynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
		pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	} else {
		pipelineCreateInfo.pDynamicState = nullptr;
	}
	
	pipelineCreateInfo.layout = GetPipelineLayout()->obj;

	// Fixed functions
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();

	// Shaders
	pipelineCreateInfo.stageCount = shaderProgram.GetStages()->size();
	pipelineCreateInfo.pStages = shaderProgram.GetStages()->data();
	
	// Create the actual pipeline
	Instance::CheckVkResult("Create Graphics Pipeline", device->CreateGraphicsPipelines(VK_NULL_HANDLE/*pipelineCache*/, 1, &pipelineCreateInfo, nullptr, obj));
}

void RasterShaderPipelineObject::Destroy() {
	if (device) {
		dynamicStates.clear();
		viewports.clear();
		scissors.clear();
		device->DestroyPipeline(obj, nullptr);
		shaderProgram.DestroyShaderStages(device);
		device = nullptr;
		pipelineCreateInfo.pViewportState = nullptr;
	}
}
