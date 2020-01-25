/*
 * Vulkan Rasterization Pipeline abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class extends from ShaderPipeline with specific functionality for rasterization
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB RasterShaderPipeline : public ShaderPipeline {
		VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
		Image* renderTarget;
	public:
		
		// Data to draw
		Buffer* vertexBuffer = nullptr;
		Buffer* indexBuffer = nullptr;
		uint32_t vertexCount = 0;
		VkDeviceSize vertexOffset = 0;
		uint32_t indexCount = 0;
		VkDeviceSize indexOffset = 0;
		
		// Graphics Pipeline information
		VkPipelineRasterizationStateCreateInfo rasterizer {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr, // const void* pNext
			0, // VkPipelineRasterizationStateCreateFlags flags
			VK_TRUE, // VkBool32 depthClampEnable
			VK_FALSE, // VkBool32 rasterizerDiscardEnable
			VK_POLYGON_MODE_FILL, // VkPolygonMode polygonMode
			VK_CULL_MODE_BACK_BIT, // VkCullModeFlags cullMode
			VK_FRONT_FACE_COUNTER_CLOCKWISE, // VkFrontFace frontFace
			VK_FALSE, // VkBool32 depthBiasEnable
			0, // float depthBiasConstantFactor
			0, // float depthBiasClamp
			0, // float depthBiasSlopeFactor
			1 // float lineWidth
		};
		VkPipelineMultisampleStateCreateInfo multisampling {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			nullptr, // const void* pNext
			0, // VkPipelineMultisampleStateCreateFlags flags
			VK_SAMPLE_COUNT_1_BIT, // VkSampleCountFlagBits rasterizationSamples
			VK_TRUE, // VkBool32 sampleShadingEnable
			0.2f, // float minSampleShading
			nullptr, // const VkSampleMask* pSampleMask
			VK_FALSE, // VkBool32 alphaToCoverageEnable
			VK_FALSE // VkBool32 alphaToOneEnable
		};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr, // const void* pNext
			0, // VkPipelineInputAssemblyStateCreateFlags flags
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // VkPrimitiveTopology topology
			VK_FALSE, // VkBool32 primitiveRestartEnable  // If set to VK_TRUE, then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.
		};
		VkPipelineDepthStencilStateCreateInfo depthStencilState {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr, // const void* pNext
			0, // VkPipelineDepthStencilStateCreateFlags flags
			VK_TRUE, // VkBool32 depthTestEnable
			VK_TRUE, // VkBool32 depthWriteEnable
			VK_COMPARE_OP_GREATER, // VkCompareOp depthCompareOp
			VK_FALSE, // VkBool32 depthBoundsTestEnable
			VK_FALSE, // VkBool32 stencilTestEnable
			{}, // VkStencilOpState front
			{}, // VkStencilOpState back
			0.0f, // float minDepthBounds
			1.0f, // float maxDepthBounds
		};
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments {};
		VkPipelineColorBlendStateCreateInfo colorBlending {};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
		std::vector<VkDynamicState> dynamicStates {}; // Dynamic settings that CAN be changed at runtime but preferably NOT every frame
		
		RasterShaderPipeline(PipelineLayout& pipelineLayout, const std::vector<ShaderInfo>& shaderInfo);
		virtual ~RasterShaderPipeline();
		
		// set what data to draw
		void SetData(Buffer* vertexBuffer, Buffer* indexBuffer, uint32_t indexCount = 0);
		void SetData(Buffer* vertexBuffer, VkDeviceSize vertexOffset, Buffer* indexBuffer, VkDeviceSize indexOffset, uint32_t indexCount);
		void SetData(Buffer* vertexBuffer, uint32_t vertexCount);
		void SetData(Buffer* vertexBuffer, VkDeviceSize vertexOffset, uint32_t vertexCount);
		void SetData(uint32_t vertexCount);

		virtual void CreatePipeline(Device* device) override;
		virtual void DestroyPipeline(Device* device) override;
		
		// assign render pass
		void SetRenderPass(VkPipelineViewportStateCreateInfo* viewportState, VkRenderPass, uint32_t subpass = 0);
		void SetRenderPass(SwapChain*, VkRenderPass, uint32_t subpass = 0);
		void SetRenderPass(Image* renderTarget, VkRenderPass, uint32_t subpass = 0);
		
		void AddColorBlendAttachmentState(
			VkBool32 blendEnable = VK_TRUE,
			VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
			VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD,
			VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		);
		
	protected:
		// these two methods are called automatically by Execute() from the parent class
		virtual void Bind(Device* device, VkCommandBuffer cmdBuffer) override;
		virtual void Render(Device* device, VkCommandBuffer cmdBuffer, uint32_t instanceCount = 1) override;
	};
	
}
