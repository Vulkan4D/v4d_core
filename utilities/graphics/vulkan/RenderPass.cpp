#include <v4d.h>

using namespace v4d::graphics::vulkan;

RenderPass::RenderPass(Device* device) : device(device) {
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
}

RenderPass::~RenderPass() {
	device->DestroyRenderPass(handle, nullptr);
}

void RenderPass::Create() {
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();

	renderPassInfo.subpassCount = subpasses.size();
	renderPassInfo.pSubpasses = subpasses.data();

	if (device->CreateRenderPass(&renderPassInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void RenderPass::AddSubpass(VkSubpassDescription& subpass) {
	subpasses.push_back(subpass);
}

uint32_t RenderPass::AddAttachment(VkAttachmentDescription& attachment) {
	uint32_t index = attachments.size();
	attachments.push_back(attachment);
	return index;
}

GraphicsPipeline* RenderPass::NewGraphicsPipeline(Device* device, uint32_t subpass) {
	auto* graphicsPipeline = new GraphicsPipeline(device);
	graphicsPipelines.push_back(graphicsPipeline);
	// Render passes 
	// The reference to the render pass and the index of the sub pass where this graphics pipeline will be used. 
	// It is also possible to use other render passes with this pipeline instead of this specific instance, but they have to be Compatible with renderPass.
	graphicsPipeline->pipelineCreateInfo.renderPass = handle;
	graphicsPipeline->pipelineCreateInfo.subpass = subpass;

	return graphicsPipeline;
}

void RenderPass::CreateGraphicsPipelines() {
	std::vector<VkGraphicsPipelineCreateInfo> pipelineCreates(graphicsPipelines.size());
	for (size_t i = 0; i < graphicsPipelines.size(); i++) {
		graphicsPipelines[i]->Prepare();
		pipelineCreates[i] = std::ref(graphicsPipelines[i]->pipelineCreateInfo);
	}

	std::vector<VkPipeline> pipelineHandles(graphicsPipelines.size());
	
	// Now create the graphics pipeline object
	// This function is designed to take multiple VkGraphicsPipelineCreateInfo objects and create multiple VkPipeline objects in a single call.
	if (device->CreateGraphicsPipelines(VK_NULL_HANDLE/*pipelineCache*/, pipelineCreates.size(), pipelineCreates.data(), nullptr, pipelineHandles.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline");
	}

	for (size_t i = 0; i < graphicsPipelines.size(); i++) {
		graphicsPipelines[i]->handle = pipelineHandles[i];
	}
}

void RenderPass::DestroyGraphicsPipelines() {
	for (auto* gp : graphicsPipelines)
		delete gp;
	graphicsPipelines.clear();
}
