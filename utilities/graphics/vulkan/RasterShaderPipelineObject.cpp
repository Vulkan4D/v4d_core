#include "RasterShaderPipelineObject.h"
#include "utilities/graphics/vulkan/Instance.h"

using namespace v4d::graphics::vulkan;

void RasterShaderPipelineObject::Create(Device* device) {
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
	vertexInputInfo.vertexBindingDescriptionCount = shaderProgram.GetBindings()->size();
	vertexInputInfo.pVertexBindingDescriptions = shaderProgram.GetBindings()->data();
	vertexInputInfo.vertexAttributeDescriptionCount = shaderProgram.GetAttributes()->size();
	vertexInputInfo.pVertexAttributeDescriptions = shaderProgram.GetAttributes()->data();

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
	assert(device);
	dynamicStates.clear();
	viewports.clear();
	scissors.clear();
	device->DestroyPipeline(obj, nullptr);
	shaderProgram.DestroyShaderStages(device);
	device = nullptr;
	pipelineCreateInfo.pViewportState = nullptr;
}

void RasterShaderPipelineObject::Render(uint32_t frameIndex, VkCommandBuffer cmdBuffer, uint32_t instanceCount) {
	assert(device);
	if (vertexBuffer[frameIndex] == VK_NULL_HANDLE) {
		device->CmdDraw(cmdBuffer,
			vertexCount, // vertexCount
			instanceCount, // instanceCount
			0, // firstVertex (defines the lowest value of gl_VertexIndex)
			0  // firstInstance (defines the lowest value of gl_InstanceIndex)
		);
	} else {
		device->CmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer[frameIndex], &vertexOffset);
		if (indexBuffer[frameIndex] == VK_NULL_HANDLE) {
			// Draw vertices
			if (vertexCount > 0) {
				device->CmdDraw(cmdBuffer,
					vertexCount, // vertexCount
					instanceCount, // instanceCount
					0, // firstVertex (defines the lowest value of gl_VertexIndex)
					0  // firstInstance (defines the lowest value of gl_InstanceIndex)
				);
			}
		} else if (indexCount > 0) {
			// Draw indices
			device->CmdBindIndexBuffer(cmdBuffer, indexBuffer[frameIndex], indexOffset, VK_INDEX_TYPE_UINT32);
			device->CmdDrawIndexed(cmdBuffer,
				indexCount, // indexCount
				instanceCount, // instanceCount
				0, // firstIndex
				0, // vertexOffset (0 because we are already taking an offseted vertex buffer)
				0  // firstInstance (defines the lowest value of gl_InstanceIndex)
			);
		}
	}
}
