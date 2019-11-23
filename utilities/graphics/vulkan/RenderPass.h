#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB RenderPass {
	private:
		Device* device;

	public:
		VkRenderPassCreateInfo renderPassInfo {};
		std::vector<VkSubpassDescription> subpasses {};
		std::vector<VkAttachmentDescription> attachments {}; // This struct defines the output data from the fragment shader (o_color)
		VkRenderPass handle = VK_NULL_HANDLE;
		std::vector<GraphicsPipeline*> graphicsPipelines {};

		RenderPass(Device* device);
		~RenderPass();

		void Create();

		void AddSubpass(VkSubpassDescription &subpass);
		uint32_t AddAttachment(VkAttachmentDescription &attachment);

		GraphicsPipeline* NewGraphicsPipeline(Device* device, uint32_t subpass = 0);

		void CreateGraphicsPipelines();
		void DestroyGraphicsPipelines();

	};
}
