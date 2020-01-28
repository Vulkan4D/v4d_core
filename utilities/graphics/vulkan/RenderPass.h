/*
 * Vulkan RenderPass abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB RenderPass {
	public:
	
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
		std::vector<VkAttachmentDescription> attachments {}; // This struct defines the output data from the fragment shader (o_color)
		std::vector<VkFramebuffer> frameBuffers {};

		VkRenderPass handle = VK_NULL_HANDLE;
		
		void Create(Device* device);
		void Destroy(Device* device);

		void AddSubpass(VkSubpassDescription &subpass);
		
		uint32_t AddAttachment(VkAttachmentDescription &attachment);
		
		// automatically create FrameBufers given specific image attachments
		void CreateFrameBuffers(Device* device, const VkExtent2D&, const VkImageView* attachments, uint32_t attachmentCount = 1, uint32_t layers = 1);
		void CreateFrameBuffers(Device* device, const VkExtent2D&, const std::vector<VkImageView>& attachments, uint32_t layers = 1);
		void CreateFrameBuffers(Device* device, SwapChain*, const VkImageView* attachments, uint32_t attachmentCount = 1, uint32_t layers = 1);
		void CreateFrameBuffers(Device* device, SwapChain*, std::vector<VkImageView> attachments = {VK_NULL_HANDLE/* VK_NULL_HANDLE ==> the SwapChain image */}, uint32_t layers = 1);
		void CreateFrameBuffers(Device* device, Image*, uint32_t imageCount = 1);
		void CreateFrameBuffers(Device* device, Image**, uint32_t imageCount = 1);
		void CreateFrameBuffers(Device* device, Image&);
		void CreateFrameBuffers(Device* device, const std::vector<Image*>&);
		
		VkFramebuffer& GetFrameBuffer(int index = 0);
		
		void DestroyFrameBuffers(Device* device);
		
		void Begin(
			Device*,
			VkCommandBuffer,
			VkOffset2D,
			VkExtent2D,
			const std::vector<VkClearValue>&,
			int imageIndex = 0,
			VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE
		);
		void Begin(
			Device*,
			VkCommandBuffer,
			SwapChain*,
			const std::vector<VkClearValue>&,
			int imageIndex = 0,
			VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE
		);
		void Begin(
			Device*,
			VkCommandBuffer,
			Image& target,
			const std::vector<VkClearValue>& = {},
			int imageIndex = 0,
			VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE
		);
		
		void End(Device* device, VkCommandBuffer commandBuffer);
		
	};
}
