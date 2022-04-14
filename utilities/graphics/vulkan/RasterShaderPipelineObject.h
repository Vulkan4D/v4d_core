/*
 * Vulkan Rasterization Pipeline abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class extends from ShaderPipelineObject with specific functionality for rasterization
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/ImageObject.h"
#include "utilities/graphics/vulkan/SwapChain.h"
#include "utilities/graphics/vulkan/ShaderPipelineObject.h"
#include "utilities/graphics/vulkan/BufferObject.h"

namespace v4d::graphics::vulkan {

	class V4DLIB RasterShaderPipelineObject : public ShaderPipelineObject {
	public:
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,// const void* pNext
			0,// VkPipelineCreateFlags flags
			0,// uint32_t stageCount
			nullptr,// const VkPipelineShaderStageCreateInfo* pStages
			&vertexInputInfo,// const VkPipelineVertexInputStateCreateInfo* pVertexInputState
			&inputAssembly,// const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState
			&tessellation,// const VkPipelineTessellationStateCreateInfo* pTessellationState
			nullptr,// const VkPipelineViewportStateCreateInfo* pViewportState
			&rasterizer,// const VkPipelineRasterizationStateCreateInfo* pRasterizationState
			&multisampling,// const VkPipelineMultisampleStateCreateInfo* pMultisampleState
			&depthStencilState,// const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState
			&colorBlending,// const VkPipelineColorBlendStateCreateInfo* pColorBlendState
			nullptr,// const VkPipelineDynamicStateCreateInfo* pDynamicState
			VK_NULL_HANDLE,// VkPipelineLayout layout
			VK_NULL_HANDLE,// VkRenderPass renderPass
			0,// uint32_t subpass
			// Optional
				// Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline. 
				// The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much functionality in common with an existing pipeline and switching between pipelines from the same parent can also be done quicker.
				// You can either specify the handle of an existing pipeline with basePipelineHandle or reference another pipeline that is about to be created by index with basePipelineIndex. 
				// These are only used if VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of VkGraphicsPipelineCreateInfo.
				VK_NULL_HANDLE, // basePipelineHandle
				-1 // basePipelineIndex
		};
		
		// Graphics Pipeline information
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
			VK_FALSE, // VkBool32 sampleShadingEnable
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
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			nullptr,//const void* pNext
			0,//VkPipelineVertexInputStateCreateFlags flags
			0,//uint32_t vertexBindingDescriptionCount
			nullptr,//const VkVertexInputBindingDescription* pVertexBindingDescriptions
			0,//uint32_t vertexAttributeDescriptionCount
			nullptr//const VkVertexInputAttributeDescription* pVertexAttributeDescriptions
		};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments {DefaultColorAttachmentBlendState()};
		VkPipelineColorBlendStateCreateInfo colorBlending {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			nullptr,//const void* pNext
			0,//VkPipelineColorBlendStateCreateFlags flags
			VK_FALSE,//VkBool32 logicOpEnable
			VK_LOGIC_OP_COPY,//VkLogicOp logicOp
			0,//uint32_t attachmentCount
			nullptr,//const VkPipelineColorBlendAttachmentState* pAttachments
			{0,0,0,0}//float blendConstants[4]
		};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			nullptr,//const void* pNext
			0,//VkPipelineDynamicStateCreateFlags flags
			0,//uint32_t dynamicStateCount
			nullptr//const VkDynamicState* pDynamicStates
		};
		
		VkPipelineTessellationStateCreateInfo tessellation {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			nullptr,//const void* pNext;
			0,//VkPipelineTessellationStateCreateFlags flags;
			3//uint32_t patchControlPoints;
		};
		
		VkPipelineViewportStateCreateInfo viewportState {};
		std::vector<VkDynamicState> dynamicStates {};
		std::vector<VkViewport> viewports {};
		std::vector<VkRect2D> scissors {};
		
		std::vector<VkVertexInputBindingDescription> bindings {};
		std::vector<VkVertexInputAttributeDescription> attributes {};
		
		using ShaderPipelineObject::ShaderPipelineObject;
		
		virtual void Create(Device* device) override;
		virtual void Destroy() override;
		
		void Reload() override {
			assert(device);
			device->DestroyPipeline(obj, nullptr);
			shaderProgram.DestroyShaderStages(device);
			ReadShaders();
			auto device = this->device; this->device = nullptr;//this->device must be nullptr when we call Create()
			Create(device);
		}

		void SetViewport(SwapChain* swapChain) {
			pipelineCreateInfo.pViewportState = &swapChain->viewportState;
		}
		void SetViewport(ImageObject* renderTarget) {
			pipelineCreateInfo.pViewportState = nullptr;
			viewports = {VkViewport{
				/*x*/ 0.0f,
				/*y*/ 0.0f,
				/*width*/ (float) renderTarget->width,
				/*height*/ (float) renderTarget->height,
				/*minDepth*/ 0.0f,
				/*maxDepth*/ float(renderTarget->imageInfo.extent.depth)
			}};
			scissors = {VkRect2D{VkOffset2D{0,0}, VkExtent2D{renderTarget->width, renderTarget->height}}};
		}
		void SetViewport(const VkExtent3D& extent) {
			pipelineCreateInfo.pViewportState = nullptr;
			viewports = {VkViewport{
				/*x*/ 0.0f,
				/*y*/ 0.0f,
				/*width*/ (float) extent.width,
				/*height*/ (float) extent.height,
				/*minDepth*/ 0.0f,
				/*maxDepth*/ float(extent.depth)
			}};
			scissors = {VkRect2D{VkOffset2D{0,0}, VkExtent2D{extent.width, extent.height}}};
		}
		void SetViewport(const VkViewport& viewport, const VkRect2D& scissor) {
			pipelineCreateInfo.pViewportState = nullptr;
			viewports = {viewport};
			scissors = {scissor};
		}
		void SetRenderPass(VkRenderPass renderPass, uint32_t subpass = 0) {
			pipelineCreateInfo.renderPass = renderPass;
			pipelineCreateInfo.subpass = subpass;
		}
		
		static VkPipelineColorBlendAttachmentState DefaultColorAttachmentBlendState(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			return {
				/*VkBool32 blendEnable*/ VK_TRUE,
				/*VkBlendFactor srcColorBlendFactor*/ VK_BLEND_FACTOR_SRC_ALPHA,
				/*VkBlendFactor dstColorBlendFactor*/ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				/*VkBlendOp colorBlendOp*/ VK_BLEND_OP_ADD,
				/*VkBlendFactor srcAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstAlphaBlendFactor*/ VK_BLEND_FACTOR_ZERO,
				/*VkBlendOp alphaBlendOp*/ VK_BLEND_OP_ADD,
				/*VkColorComponentFlags colorWriteMask*/ colorWriteMask
			};
		}
		
		static VkPipelineColorBlendAttachmentState ColorAttachmentNoBlendState(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			VkPipelineColorBlendAttachmentState blendState {};
				blendState.blendEnable = VK_FALSE;
				blendState.colorWriteMask = colorWriteMask;
			return blendState;
		}
		
		static VkPipelineColorBlendAttachmentState LightingAttachmentBlendState(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			return {
				/*VkBool32 blendEnable*/ VK_TRUE,
				/*VkBlendFactor srcColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp colorBlendOp*/ VK_BLEND_OP_ADD,
				/*VkBlendFactor srcAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp alphaBlendOp*/ VK_BLEND_OP_MAX,
				/*VkColorComponentFlags colorWriteMask*/ colorWriteMask
			};
		}
		
		static VkPipelineColorBlendAttachmentState AttachmentBlendStateAdd(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			return {
				/*VkBool32 blendEnable*/ VK_TRUE,
				/*VkBlendFactor srcColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp colorBlendOp*/ VK_BLEND_OP_ADD,
				/*VkBlendFactor srcAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp alphaBlendOp*/ VK_BLEND_OP_ADD,
				/*VkColorComponentFlags colorWriteMask*/ colorWriteMask
			};
		}
		
		static VkPipelineColorBlendAttachmentState AttachmentBlendStateMax(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			return {
				/*VkBool32 blendEnable*/ VK_TRUE,
				/*VkBlendFactor srcColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstColorBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp colorBlendOp*/ VK_BLEND_OP_MAX,
				/*VkBlendFactor srcAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendFactor dstAlphaBlendFactor*/ VK_BLEND_FACTOR_ONE,
				/*VkBlendOp alphaBlendOp*/ VK_BLEND_OP_MAX,
				/*VkColorComponentFlags colorWriteMask*/ colorWriteMask
			};
		}
		
		void SetColorBlendAttachmentStates(int colorAttachmentCount, VkPipelineColorBlendAttachmentState colorAttachmentBlendState = DefaultColorAttachmentBlendState()) {
			colorBlendAttachments.clear();
			for (int i = 0; i < colorAttachmentCount; ++i)
				colorBlendAttachments.push_back(colorAttachmentBlendState);
		}
		
		void SetColorBlendAttachmentStates(const std::vector<VkPipelineColorBlendAttachmentState>& colorAttachmentsBlendings) {
			colorBlendAttachments = colorAttachmentsBlendings;
		}
		
		virtual void Bind(VkCommandBuffer cmdBuffer) override {
			assert(device);
			device->CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, obj);
			GetPipelineLayout()->Bind(cmdBuffer);
		}
		
		virtual void Draw(VkCommandBuffer cmdBuffer, uint32_t vertexOrIndexCount = 0, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) {
			assert(device);
			if (vertexOrIndexCount == 0) {
				switch (inputAssembly.topology) {
					case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
						vertexOrIndexCount = 1;
					break;
					case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
					case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
						vertexOrIndexCount = 2;
					break;
					case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
					case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY: 
						vertexOrIndexCount = 4;
					break;
					case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
					case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
					case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
						vertexOrIndexCount = 3;
					break;
					case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
					case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
						vertexOrIndexCount = 6;
					break;
					case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
						vertexOrIndexCount = 0; // should trigger an error
					break;
				}
			}
			device->CmdDraw(cmdBuffer,
				vertexOrIndexCount,
				instanceCount,
				firstVertex, // (defines the lowest value of gl_VertexIndex)
				firstInstance // (defines the lowest value of gl_InstanceIndex)
			);
		}
	};
	
}
