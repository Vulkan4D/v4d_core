#include "RasterShaderPipeline.h"

using namespace v4d::graphics::vulkan;

RasterShaderPipeline::~RasterShaderPipeline() {} // dont delete anything here, we are copying/moving this object in some modules

void RasterShaderPipeline::SetData(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
	this->vertexCount = 0;
	this->vertexOffset = 0;
	this->indexCount = indexCount;
	this->indexOffset = 0;
}

void RasterShaderPipeline::SetData(VkBuffer vertexBuffer, VkDeviceSize vertexOffset, VkBuffer indexBuffer, VkDeviceSize indexOffset, uint32_t indexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
	this->vertexCount = 0;
	this->vertexOffset = vertexOffset;
	this->indexCount = indexCount;
	this->indexOffset = indexOffset;
}

void RasterShaderPipeline::SetData(VkBuffer vertexBuffer, uint32_t vertexCount) {
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = nullptr;
	this->vertexCount = vertexCount;
	this->vertexOffset = 0;
	this->indexCount = 0;
	this->indexOffset = 0;
}

void RasterShaderPipeline::SetData(uint32_t vertexCount) {
	this->vertexBuffer = nullptr;
	this->indexBuffer = nullptr;
	this->vertexCount = vertexCount;
	this->vertexOffset = 0;
	this->indexCount = 0;
	this->indexOffset = 0;
}

void RasterShaderPipeline::CreatePipeline(Device* device) {
	CreateShaderStages(device);
	
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
	vertexInputInfo.vertexBindingDescriptionCount = GetBindings()->size();
	vertexInputInfo.pVertexBindingDescriptions = GetBindings()->data();
	vertexInputInfo.vertexAttributeDescriptionCount = GetAttributes()->size();
	vertexInputInfo.pVertexAttributeDescriptions = GetAttributes()->data();

	// Dynamic states
	if (dynamicStates.size() > 0) {
		dynamicStateCreateInfo.dynamicStateCount = (uint)dynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
		pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	} else {
		pipelineCreateInfo.pDynamicState = nullptr;
	}
	
	pipelineCreateInfo.layout = GetPipelineLayout()->handle;

	// Fixed functions
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();

	// Shaders
	pipelineCreateInfo.stageCount = GetStages()->size();
	pipelineCreateInfo.pStages = GetStages()->data();
	
	// Create the actual pipeline
	if (device->CreateGraphicsPipelines(VK_NULL_HANDLE/*pipelineCache*/, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline");
	}
}

void RasterShaderPipeline::DestroyPipeline(Device* device) {
	dynamicStates.clear();
	viewports.clear();
	scissors.clear();
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
	colorBlendAttachments.clear();
}

void RasterShaderPipeline::ReloadPipeline(Device* device) {
	device->DestroyPipeline(pipeline, nullptr);
	DestroyShaderStages(device);
	ReadShaders();
	CreatePipeline(device);
}

void RasterShaderPipeline::SetRenderPass(VkPipelineViewportStateCreateInfo* viewportState, VkRenderPass renderPass, uint32_t subpass) {
	pipelineCreateInfo.pViewportState = viewportState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = subpass;
}

void RasterShaderPipeline::SetRenderPass(SwapChain* swapChain, VkRenderPass renderPass, uint32_t subpass) {
	pipelineCreateInfo.pViewportState = &swapChain->viewportState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = subpass;
}

void RasterShaderPipeline::SetRenderPass(Image* renderTarget, VkRenderPass renderPass, uint32_t subpass) {
	
	viewports.emplace_back(
		/*x*/ 0,
		/*y*/ 0,
		/*width*/ (float) renderTarget->width,
		/*height*/ (float) renderTarget->height,
		/*minDepth*/ 0,
		/*maxDepth*/ renderTarget->imageInfo.extent.depth
	);
	scissors.emplace_back(VkOffset2D{0,0}, VkExtent2D{renderTarget->width, renderTarget->height});
	
	pipelineCreateInfo.pViewportState = nullptr;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = subpass;
}

void RasterShaderPipeline::AddColorBlendAttachmentState(
	VkBool32 blendEnable,
	VkBlendFactor srcColorBlendFactor,
	VkBlendFactor dstColorBlendFactor,
	VkBlendOp colorBlendOp,
	VkBlendFactor srcAlphaBlendFactor,
	VkBlendFactor dstAlphaBlendFactor,
	VkBlendOp alphaBlendOp,
	VkColorComponentFlags colorWriteMask
) {
	colorBlendAttachments.push_back({
		blendEnable, // VkBool32
		srcColorBlendFactor, // VkBlendFactor
		dstColorBlendFactor, // VkBlendFactor
		colorBlendOp, // VkBlendOp
		srcAlphaBlendFactor, // VkBlendFactor
		dstAlphaBlendFactor, // VkBlendFactor
		alphaBlendOp, // VkBlendOp
		colorWriteMask // VkColorComponentFlags
	});
}

void RasterShaderPipeline::Bind(Device* device, VkCommandBuffer cmdBuffer) {
	device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	GetPipelineLayout()->Bind(device, cmdBuffer);
}

void RasterShaderPipeline::Render(Device* device, VkCommandBuffer cmdBuffer, uint32_t instanceCount) {
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
