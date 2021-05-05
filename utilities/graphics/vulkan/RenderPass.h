/*
 * Vulkan RenderPass abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/Instance.h"
#include "utilities/graphics/vulkan/SwapChain.h"

namespace v4d::graphics::vulkan {
	
	class RenderPass {
	
		VkRenderPassCreateInfo renderPassInfo {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,// const void* pNext
			0,// VkRenderPassCreateFlags flags
			0,// uint32_t attachmentCount
			nullptr,// const VkAttachmentDescription* pAttachments
			0,// uint32_t subpassCount
			nullptr,// const VkSubpassDescription* pSubpasses
			0,// uint32_t dependencyCount
			nullptr// const VkSubpassDependency* pDependencies
		};
		
		std::vector<VkSubpassDescription> subpasses {};
		std::vector<VkSubpassDependency> subpassDependencies {};
		std::vector<VkAttachmentDescription> attachments {}; // This struct defines the output data from the fragment shader (o_color)
		std::vector<VkClearValue> clearValues {};
		std::vector<VkFramebuffer> frameBuffers {};
		std::vector<std::vector<VkImageView>> imageViews {};

		Device* device = nullptr;
		uint32_t renderWidth = 0;
		uint32_t renderHeight = 0;
		
	public:
		VkRenderPass handle = VK_NULL_HANDLE;
		
		void Create(Device* device, uint32_t width, uint32_t height, uint32_t layers = 1) {
			this->device = device;
			this->renderWidth = width;
			this->renderHeight = height;
			
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pAttachments = attachments.data();

			renderPassInfo.subpassCount = subpasses.size();
			renderPassInfo.pSubpasses = subpasses.data();

			renderPassInfo.dependencyCount = subpassDependencies.size();
			renderPassInfo.pDependencies = subpassDependencies.data();
			
			if (device->CreateRenderPass(&renderPassInfo, nullptr, &handle) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass!");
			}
			
			// Create FrameBuffers
			VkFramebufferCreateInfo framebufferCreateInfo = {};
				framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCreateInfo.renderPass = handle;
				framebufferCreateInfo.attachmentCount = renderPassInfo.attachmentCount;
				framebufferCreateInfo.width = width;
				framebufferCreateInfo.height = height;
				framebufferCreateInfo.layers = layers;
			
			for (size_t i = 0; i < frameBuffers.size(); ++i) {
				framebufferCreateInfo.pAttachments = imageViews[i].data();
				Instance::CheckVkResult("Create framebuffer", device->CreateFramebuffer(&framebufferCreateInfo, nullptr, &frameBuffers[i]));
			}
		}

		void Destroy() {
			if (!device) return;
			
			for (auto framebuffer : frameBuffers) {
				device->DestroyFramebuffer(framebuffer, nullptr);
			}
			
			device->DestroyRenderPass(handle, nullptr);
			frameBuffers.clear();
			subpasses.clear();
			subpassDependencies.clear();
			attachments.clear();
			clearValues.clear();
			imageViews.clear();
			
			colorAttachmentRefs.clear();
			inputAttachmentRefs.clear();
			resolveAttachmentRefs.clear();
			preserveAttachmentRefs.clear();
			depthStencilAttachmentRefs.clear();
			
			device = nullptr;
		}
		
		void SetFrameBufferCount(uint n) {
			frameBuffers.resize(n);
			imageViews.resize(n);
		}
		
		struct Subpass {
			VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			std::vector<VkAttachmentReference>* inputAttachments = nullptr;
			std::vector<VkAttachmentReference>* colorAttachments = nullptr;
			std::vector<VkAttachmentReference>* resolveAttachments = nullptr;
			std::vector<uint32_t>* preserveAttachments = nullptr;
			VkAttachmentReference* depthStencilAttachment = nullptr;
			VkSubpassDescriptionFlags flags;
			
			operator VkSubpassDescription() {
				VkSubpassDescription subpass {};
					subpass.flags = flags; // VkSubpassDescriptionFlags
					subpass.pipelineBindPoint = pipelineBindPoint; // VkPipelineBindPoint
					subpass.inputAttachmentCount = inputAttachments? inputAttachments->size() : 0; // uint32_t
					subpass.pInputAttachments = inputAttachments? inputAttachments->data() : nullptr; // const VkAttachmentReference*
					subpass.colorAttachmentCount = colorAttachments? colorAttachments->size() : 0; // uint32_t
					subpass.pColorAttachments = colorAttachments? colorAttachments->data() : nullptr; // const VkAttachmentReference*
					subpass.pResolveAttachments = resolveAttachments? resolveAttachments->data() : nullptr; // const VkAttachmentReference*
					subpass.pDepthStencilAttachment = depthStencilAttachment; // const VkAttachmentReference*
					subpass.preserveAttachmentCount = preserveAttachments? preserveAttachments->size() : 0; // uint32_t
					subpass.pPreserveAttachments = preserveAttachments? preserveAttachments->data() : nullptr; // const uint32_t*
				return subpass;
			}
		};
		
		std::vector<std::unique_ptr<std::vector<VkAttachmentReference>>> colorAttachmentRefs {};
		std::vector<std::unique_ptr<std::vector<VkAttachmentReference>>> inputAttachmentRefs {};
		std::vector<std::unique_ptr<std::vector<VkAttachmentReference>>> resolveAttachmentRefs {};
		std::vector<std::unique_ptr<std::vector<uint32_t>>> preserveAttachmentRefs {};
		std::vector<std::unique_ptr<VkAttachmentReference>> depthStencilAttachmentRefs {};
		
		std::vector<VkAttachmentReference>* AddColorAttachmentRefs(const std::vector<VkAttachmentReference>& refs) {
			return colorAttachmentRefs.emplace_back(new std::vector<VkAttachmentReference>(refs)).get();
		}
		std::vector<VkAttachmentReference>* AddInputAttachmentRefs(const std::vector<VkAttachmentReference>& refs) {
			return inputAttachmentRefs.emplace_back(new std::vector<VkAttachmentReference>(refs)).get();
		}
		std::vector<VkAttachmentReference>* AddResolveAttachmentRefs(const std::vector<VkAttachmentReference>& refs) {
			return resolveAttachmentRefs.emplace_back(new std::vector<VkAttachmentReference>(refs)).get();
		}
		std::vector<uint32_t>* AddPreserveAttachmentRefs(const std::vector<uint32_t>& refs) {
			return preserveAttachmentRefs.emplace_back(new std::vector<uint32_t>(refs)).get();
		}
		VkAttachmentReference* AddPreserveAttachmentRefs(const VkAttachmentReference& ref) {
			return depthStencilAttachmentRefs.emplace_back(new VkAttachmentReference{ref.attachment, ref.layout}).get();
		}

		uint32_t AddSubpass(const VkSubpassDescription& subpass, int32_t srcSubpassDependency = -1, 
			// Subpass Dependency
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkAccessFlags srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VkDependencyFlags dependencyFlags = 0
		){
			uint32_t index = subpasses.size();
			subpasses.push_back(subpass);
			if (srcSubpassDependency != -1) {
				VkSubpassDependency subPassDependency {
					/*srcSubpass*/uint32_t(srcSubpassDependency),
					/*dstSubpass*/index,
					srcStageMask,
					dstStageMask,
					srcAccessMask,
					dstAccessMask,
					dependencyFlags
				};
			}
			return index;
		}
		
		void AddSubpassDependency(const VkSubpassDependency& subpassDependency) {
			subpassDependencies.push_back(subpassDependency);
		}
		
		[[nodiscard]] uint32_t AddAttachment(
			const std::vector<VkImageView>& frameBuffered_imageView,
			VkFormat format,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout,
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VkAttachmentDescriptionFlags flags = 0,
			VkClearValue clear = {0,0,0,0}
		) {
			assert(frameBuffered_imageView.size() > 0);
			assert(frameBuffers.size() >= frameBuffered_imageView.size());
			assert(imageViews.size() == frameBuffers.size());
			
			for (size_t i = 0; i < imageViews.size(); ++i) {
				imageViews[i].emplace_back(frameBuffered_imageView[i % frameBuffered_imageView.size()]);
			}
			
			uint32_t index = attachments.size();
			
			attachments.emplace_back(
				flags,
				format,
				samples,
				loadOp, storeOp,
				stencilLoadOp, stencilStoreOp,
				initialLayout, finalLayout
			);
			
			clearValues.emplace_back(clear);
			
			return index;
		}
		
		[[nodiscard]] uint32_t AddAttachment(
			const std::vector<const Image*>& frameBuffered_imagesPtr,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout,
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VkAttachmentDescriptionFlags flags = 0,
			VkClearValue clear = {0,0,0,0}
		) {
			assert(frameBuffered_imagesPtr.size() > 0);
			std::vector<VkImageView> imageViews {};
			for (auto& i : frameBuffered_imagesPtr) imageViews.push_back(i->view);
			return AddAttachment(imageViews, frameBuffered_imagesPtr[0]->format, initialLayout,finalLayout,samples,loadOp,storeOp,stencilLoadOp,stencilStoreOp,flags,clear);
		}
		
		[[nodiscard]] uint32_t AddAttachment(
			const VkImageView& imageView,
			VkFormat format,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout,
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VkAttachmentDescriptionFlags flags = 0,
			VkClearValue clear = {0,0,0,0}
		) {
			return AddAttachment({imageView}, format, initialLayout,finalLayout,samples,loadOp,storeOp,stencilLoadOp,stencilStoreOp,flags,clear);
		}
		
		[[nodiscard]] uint32_t AddAttachment(
			const Image& image,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout,
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VkAttachmentDescriptionFlags flags = 0,
			VkClearValue clear = {0,0,0,0}
		) {
			return AddAttachment({&image}, initialLayout,finalLayout,samples,loadOp,storeOp,stencilLoadOp,stencilStoreOp,flags,clear);
		}
		
		[[nodiscard]] uint32_t AddAttachment(
			SwapChain* swapChain,
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VkAttachmentDescriptionFlags flags = 0,
			VkClearValue clear = {0,0,0,0}
		) {
			assert(swapChain->imageViews.size() > 0);
			std::vector<VkImageView> imageViews {};
			for (auto& i : swapChain->imageViews) imageViews.push_back(i);
			return AddAttachment(imageViews, swapChain->format.format, VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,samples,loadOp,storeOp,stencilLoadOp,stencilStoreOp,flags,clear);
		}

		void Begin(const VkCommandBuffer& commandBuffer, int imageIndex = 0, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) {
			VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = handle;
				renderPassInfo.framebuffer = frameBuffers.size()>size_t(imageIndex)? frameBuffers[imageIndex] : frameBuffers[imageIndex % frameBuffers.size()];
				renderPassInfo.renderArea.offset = {0,0};
				renderPassInfo.renderArea.extent = {renderWidth, renderHeight};
				renderPassInfo.clearValueCount = clearValues.size();
				renderPassInfo.pClearValues = clearValues.data();
			device->CmdBeginRenderPass(commandBuffer, &renderPassInfo, contents);
		}

		void End(const VkCommandBuffer& commandBuffer) {
			device->CmdEndRenderPass(commandBuffer);
		}
		
	};
}
