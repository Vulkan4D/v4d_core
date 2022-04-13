#include "RenderPassObject.h"

using namespace v4d::graphics::vulkan;

COMMON_OBJECT_CPP(RenderPassObject, VkRenderPass)

void RenderPassObject::Create(Device* device) {
	assert(this->device == nullptr);
	this->device = device;

	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	
	renderPassInfo.subpassCount = subpasses.size();
	renderPassInfo.pSubpasses = subpasses.data();

	renderPassInfo.dependencyCount = subpassDependencies.size();
	renderPassInfo.pDependencies = subpassDependencies.data();
	
	if (device->CreateRenderPass(&renderPassInfo, nullptr, obj) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
	
	for (auto&[shader, subpass] : shaders) {
		shader->SetRenderPass(obj, subpass);
	}
	
	// Create Framebuffer
	VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = obj;
		framebufferCreateInfo.width = renderWidth;
		framebufferCreateInfo.height = renderHeight;
		framebufferCreateInfo.layers = renderLayers;
		framebufferCreateInfo.attachmentCount = renderPassInfo.attachmentCount;
	VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo {};
	if (imageless) {
		assert(attachments.size() == framebufferAttachments.size());
		framebufferAttachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
		framebufferAttachmentsCreateInfo.attachmentImageInfoCount = framebufferAttachments.size();
		framebufferAttachmentsCreateInfo.pAttachmentImageInfos = framebufferAttachments.data();
		framebufferCreateInfo.pNext = &framebufferAttachmentsCreateInfo;
		framebufferCreateInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
	}
	
	if (imageless) {
		framebufferCreateInfo.pAttachments = nullptr;
	} else {
		assert(framebufferCreateInfo.attachmentCount == imageViews.size());
		framebufferCreateInfo.pAttachments = imageViews.data();
	}
	Instance::CheckVkResult("Create framebuffer", device->CreateFramebuffer(&framebufferCreateInfo, nullptr, &framebuffer));
}

void RenderPassObject::Destroy() {
	if (device) {
		device->DestroyFramebuffer(framebuffer, nullptr);
		device->DestroyRenderPass(obj, nullptr);
		subpasses.clear();
		subpassDependencies.clear();
		attachments.clear();
		clearValues.clear();
		imageViews.clear();
		framebufferAttachments.clear();
		framebufferAttachmentsFormats.clear();
		
		colorAttachmentRefs.clear();
		inputAttachmentRefs.clear();
		resolveAttachmentRefs.clear();
		preserveAttachmentRefs.clear();
		depthStencilAttachmentRefs.clear();
		
		shaders.clear();
		
		device = nullptr;
	}
}
