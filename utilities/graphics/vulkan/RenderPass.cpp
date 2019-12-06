#include <v4d.h>

using namespace v4d::graphics::vulkan;

void RenderPass::Create(Device* device) {
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();

	renderPassInfo.subpassCount = subpasses.size();
	renderPassInfo.pSubpasses = subpasses.data();

	if (device->CreateRenderPass(&renderPassInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void RenderPass::Destroy(Device* device) {
	device->DestroyRenderPass(handle, nullptr);
	subpasses.clear();
	attachments.clear();
}

void RenderPass::AddSubpass(VkSubpassDescription& subpass) {
	subpasses.push_back(subpass);
}

uint32_t RenderPass::AddAttachment(VkAttachmentDescription& attachment) {
	uint32_t index = attachments.size();
	attachments.push_back(attachment);
	return index;
}

VkFramebuffer& RenderPass::GetFrameBuffer(int index) {
	if (frameBuffers.size() < index+1) frameBuffers.resize(index+1);
	return frameBuffers[index];
}
