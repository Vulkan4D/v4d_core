#include "RasterShaderPipelineObject.h"

using namespace v4d::graphics::vulkan;

void RasterShaderPipelineObject::SetData(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
	this->vertexCount = 0;
	this->vertexOffset = 0;
	this->indexCount = indexCount;
	this->indexOffset = 0;
}

void RasterShaderPipelineObject::SetData(VkBuffer vertexBuffer, VkDeviceSize vertexOffset, VkBuffer indexBuffer, VkDeviceSize indexOffset, uint32_t indexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
	this->vertexCount = 0;
	this->vertexOffset = vertexOffset;
	this->indexCount = indexCount;
	this->indexOffset = indexOffset;
}

void RasterShaderPipelineObject::SetData(VkBuffer vertexBuffer, uint32_t vertexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = nullptr;
	this->vertexCount = vertexCount;
	this->vertexOffset = 0;
	this->indexCount = 0;
	this->indexOffset = 0;
}

void RasterShaderPipelineObject::SetData(uint32_t vertexCount) {
	this->vertexBuffer = nullptr;
	this->indexBuffer = nullptr;
	this->vertexCount = vertexCount;
	this->vertexOffset = 0;
	this->indexCount = 0;
	this->indexOffset = 0;
}

void RasterShaderPipelineObject::Create(Device* device) {
	shaderProgram.CreateShaderStages(device);
	
	VkPipelineViewportStateCreateInfo viewportState {};
	
	if (pipelineCreateInfo.pViewportState == nullptr) {
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = viewports.size();
		viewportState.scissorCount = scissors.size();
		viewportState.pViewports = viewports.size()>0? viewports.data() : nullptr;
		viewportState.pScissors = scissors.size()>0? scissors.data() : nullptr;
		pipelineCreateInfo.pViewportState = &viewportState;
	}
	
	// if (viewports.size() == 0) {
	// 	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	// }
	
	// if (scissors.size() == 0) {
	// 	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	// }
	
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
	if (device->CreateGraphicsPipelines(VK_NULL_HANDLE/*pipelineCache*/, 1, &pipelineCreateInfo, nullptr, obj) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline");
	}
}

void RasterShaderPipelineObject::Destroy(Device* device) {
	dynamicStates.clear();
	viewports.clear();
	scissors.clear();
	device->DestroyPipeline(obj, nullptr);
	shaderProgram.DestroyShaderStages(device);
}

void RasterShaderPipelineObject::Reload(Device* device) {
	device->DestroyPipeline(obj, nullptr);
	shaderProgram.DestroyShaderStages(device);
	ReadShaders();
	Create(device);
}

void RasterShaderPipelineObject::Bind(Device* device, VkCommandBuffer cmdBuffer) {
	device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, obj);
	GetPipelineLayout()->Bind(device, cmdBuffer);
}

void RasterShaderPipelineObject::Render(Device* device, VkCommandBuffer cmdBuffer, uint32_t instanceCount) {
	if (vertexBuffer == VK_NULL_HANDLE) {
		device->CmdDraw(cmdBuffer,
			vertexCount, // vertexCount
			instanceCount, // instanceCount
			0, // firstVertex (defines the lowest value of gl_VertexIndex)
			0  // firstInstance (defines the lowest value of gl_InstanceIndex)
		);
	} else {
		device->CmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer, &vertexOffset);
		if (indexBuffer == VK_NULL_HANDLE) {
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
			device->CmdBindIndexBuffer(cmdBuffer, indexBuffer, indexOffset, VK_INDEX_TYPE_UINT32);
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
