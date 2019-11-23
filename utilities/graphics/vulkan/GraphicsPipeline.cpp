#include <v4d.h>

using namespace v4d::graphics::vulkan;

GraphicsPipeline::GraphicsPipeline(Device* device) : device(device) {
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	colorBlending.logicOpEnable = VK_FALSE; // If enabled, will effectively replace and disable blendingAttachmentState.blendEnable
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // optional, if enabled above
	colorBlending.blendConstants[0] = 0; // optional
	colorBlending.blendConstants[1] = 0; // optional
	colorBlending.blendConstants[2] = 0; // optional
	colorBlending.blendConstants[3] = 0; // optional
	
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	
	// Optional
	// Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline. 
	// The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much functionality in common with an existing pipeline and switching between pipelines from the same parent can also be done quicker.
	// You can either specify the handle of an existing pipeline with basePipelineHandle or reference another pipeline that is about to be created by index with basePipelineIndex. 
	// These are only used if VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of VkGraphicsPipelineCreateInfo.
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
}

GraphicsPipeline::~GraphicsPipeline() {
	device->DestroyPipeline(handle, nullptr);
}

void GraphicsPipeline::Prepare() {

	// Bindings and Attributes
	vertexInputInfo.vertexBindingDescriptionCount = bindings->size();
	vertexInputInfo.pVertexBindingDescriptions = bindings->data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributes->size();
	vertexInputInfo.pVertexAttributeDescriptions = attributes->data();

	// Dynamic states
	if (dynamicStates.size() > 0) {
		dynamicStateCreateInfo.dynamicStateCount = (uint)dynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
		pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	} else {
		pipelineCreateInfo.pDynamicState = nullptr;
	}
	
	pipelineCreateInfo.layout = pipelineLayout->handle;

	// Fixed functions
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();
	pipelineCreateInfo.pColorBlendState = &colorBlending;

	// Shaders
	pipelineCreateInfo.stageCount = shaderStages->size();
	pipelineCreateInfo.pStages = shaderStages->data();
}

void GraphicsPipeline::Create() {
	if (device->CreateGraphicsPipelines(VK_NULL_HANDLE/*pipelineCache*/, 1, &pipelineCreateInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline");
	}
}

void GraphicsPipeline::SetShaderProgram(ShaderProgram* shaderProgram) {
	shaderStages = shaderProgram->GetStages();
	bindings = shaderProgram->GetBindings();
	attributes = shaderProgram->GetAttributes();
	pipelineLayout = shaderProgram->GetPipelineLayout();
}

void GraphicsPipeline::Bind(Device* device, VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) {
	device->CmdBindPipeline(commandBuffer, bindPoint, handle);
	pipelineLayout->Bind(device, commandBuffer);
}

void GraphicsPipeline::AddColorBlendAttachmentState(VkBool32 blendEnable, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp, VkColorComponentFlags colorWriteMask) {
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
