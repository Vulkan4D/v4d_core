#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB GraphicsPipeline {
	private:
		Device* device;

		std::vector<VkPipelineShaderStageCreateInfo>* shaderStages;
		std::vector<VkVertexInputBindingDescription>* bindings;
		std::vector<VkVertexInputAttributeDescription>* attributes;

	public:
		VkPipeline handle = VK_NULL_HANDLE;
		PipelineLayout* pipelineLayout;

		VkPipelineRasterizationStateCreateInfo rasterizer {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr, // const void* pNext
			0, // VkPipelineRasterizationStateCreateFlags flags
			VK_FALSE, // VkBool32 depthClampEnable
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
			VK_COMPARE_OP_LESS, // VkCompareOp depthCompareOp
			VK_FALSE, // VkBool32 depthBoundsTestEnable
			VK_FALSE, // VkBool32 stencilTestEnable
			{}, // VkStencilOpState front
			{}, // VkStencilOpState back
			0.0f, // float minDepthBounds
			1.0f, // float maxDepthBounds
		};
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments {};
		VkPipelineColorBlendStateCreateInfo colorBlending {};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
		std::vector<VkDynamicState> dynamicStates {}; // Dynamic settings that CAN be changed at runtime but NOT every frame

		GraphicsPipeline(Device* device);
		~GraphicsPipeline();

		void Prepare();

		void Create();

		void SetShaderProgram(ShaderProgram* shaderProgram);

		void Bind(Device* device, VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

		void AddAlphaBlendingAttachment();
		void AddColorAddAttachment();
		void AddOpaqueAttachment();
		void AddOitAttachments();
		
	};
}
