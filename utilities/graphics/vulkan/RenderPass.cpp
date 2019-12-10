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

void RenderPass::CreateFrameBuffers(Device* device, SwapChain* swapChain, std::vector<VkImageView> attachments, uint32_t layers) {
	VkImageView* swapChainImageView = nullptr;
	for (auto& attachment : attachments) {
		if (attachment == VK_NULL_HANDLE) {
			swapChainImageView = &attachment;
		}
	}
	frameBuffers.resize(swapChain->imageViews.size());
	for (size_t i = 0; i < swapChain->imageViews.size(); i++) {
		if (swapChainImageView) {
			*swapChainImageView = swapChain->imageViews[i];
		}
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = handle;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapChain->extent.width;
		framebufferCreateInfo.height = swapChain->extent.height;
		framebufferCreateInfo.layers = layers;
		if (device->CreateFramebuffer(&framebufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
}

void RenderPass::CreateFrameBuffers(Device* device, const VkExtent2D& extent, const std::vector<VkImageView>& attachments, uint32_t layers) {
	frameBuffers.resize(1);
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = handle;
	framebufferCreateInfo.attachmentCount = attachments.size();
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = extent.width;
	framebufferCreateInfo.height = extent.height;
	framebufferCreateInfo.layers = layers;
	if (device->CreateFramebuffer(&framebufferCreateInfo, nullptr, &frameBuffers[0]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer");
	}
}

void RenderPass::CreateFrameBuffers(Device* device, const std::vector<Image*>& images) {
	std::vector<VkImageView> attachments {};
	for (auto* image : images) attachments.push_back(image->view);
	CreateFrameBuffers(device, {images[0]->width, images[0]->height}, attachments, images[0]->arrayLayers);
}

void RenderPass::CreateFrameBuffers(Device* device, Image& image) {
	CreateFrameBuffers(device, {image.width, image.height}, {image.view}, image.arrayLayers);
}

void RenderPass::DestroyFrameBuffers(Device* device) {
	for (auto framebuffer : frameBuffers) {
		device->DestroyFramebuffer(framebuffer, nullptr);
	}
}

void RenderPass::Begin(Device* device, VkCommandBuffer commandBuffer, VkOffset2D offset, VkExtent2D extent, const std::vector<VkClearValue>& clearValues, int imageIndex, VkSubpassContents contents) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = handle;
	renderPassInfo.framebuffer = GetFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = offset;
	renderPassInfo.renderArea.extent = extent;
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();
	device->CmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::Begin(Device* device, VkCommandBuffer commandBuffer, SwapChain* swapChain, const std::vector<VkClearValue>& clearValues, int imageIndex, VkSubpassContents contents) {
	Begin(device, commandBuffer, {0,0}, swapChain->extent, clearValues, imageIndex, contents);
}

void RenderPass::Begin(Device* device, VkCommandBuffer commandBuffer, Image& target, const std::vector<VkClearValue>& clearValues, int imageIndex, VkSubpassContents contents) {
	Begin(device, commandBuffer, {0,0}, {target.width, target.height}, clearValues, imageIndex, contents);
}

void RenderPass::End(Device* device, VkCommandBuffer commandBuffer) {
	device->CmdEndRenderPass(commandBuffer);
}
