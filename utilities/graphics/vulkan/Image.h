/*
 * Vulkan Image abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class contains abstractions for VkImage, VkImageView and VkImageSampler
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics::vulkan {
	class V4DLIB ImageObject {
		COMMON_OBJECT(ImageObject, VkImage, V4DLIB)
		
		// Constructor args
		VkImageUsageFlags usage;
		uint32_t mipLevels;
		uint32_t arrayLayers;
		std::vector<VkFormat> preferredFormats;
		
		// After Create()
		uint32_t width = 0;
		uint32_t height = 0;
		VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		MemoryAllocation allocation = VK_NULL_HANDLE;
		
		int formatFeatures = 0;
		uint32_t squareSize = 0;
		float scale = 1.0;
		
		Device* device = nullptr;
		
		ImageObject(
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			uint32_t mipLevels = 1,
			uint32_t arrayLayers = 1,
			const std::vector<VkFormat>& preferredFormats = {VK_FORMAT_R32G32B32A32_SFLOAT},
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D
		);
		
		virtual ~ImageObject();
		
		VkImageCreateInfo imageInfo {
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,// const void* pNext
			0,// VkImageCreateFlags flags
			VK_IMAGE_TYPE_2D,// VkImageType imageType
			VK_FORMAT_R32G32B32A32_SFLOAT,// VkFormat format
			{0,0,1},// VkExtent3D extent
			1,// uint32_t mipLevels
			1,// uint32_t arrayLayers
			VK_SAMPLE_COUNT_1_BIT,// VkSampleCountFlagBits samples
			VK_IMAGE_TILING_OPTIMAL,// VkImageTiling tiling
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,// VkImageUsageFlags usage
			VK_SHARING_MODE_EXCLUSIVE,// VkSharingMode sharingMode
			0,// uint32_t queueFamilyIndexCount
			nullptr,// const uint32_t* pQueueFamilyIndices
			VK_IMAGE_LAYOUT_UNDEFINED// VkImageLayout initialLayout
		};
		VkImageViewCreateInfo viewInfo {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,// VkStructureType sType
			nullptr,// const void* pNext
			0,// VkImageViewCreateFlags flags
			VK_NULL_HANDLE,// VkImage image
			VK_IMAGE_VIEW_TYPE_2D,// VkImageViewType viewType
			VK_FORMAT_R32G32B32A32_SFLOAT,// VkFormat format
			{// VkComponentMapping components
				VK_COMPONENT_SWIZZLE_R, 
				VK_COMPONENT_SWIZZLE_G, 
				VK_COMPONENT_SWIZZLE_B, 
				VK_COMPONENT_SWIZZLE_A
			},
			{// VkImageSubresourceRange subresourceRange
				VK_IMAGE_ASPECT_COLOR_BIT,// VkImageAspectFlags aspectMask
				0,// uint32_t baseMipLevel
				1,// uint32_t levelCount
				0,// uint32_t baseArrayLayer
				1// uint32_t layerCount
			}
		};
		VkSamplerCreateInfo samplerInfo {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,// VkStructureType sType
			nullptr,// const void* pNext
			0,// VkSamplerCreateFlags flags
			VK_FILTER_LINEAR,// VkFilter magFilter
			VK_FILTER_LINEAR,// VkFilter minFilter
			VK_SAMPLER_MIPMAP_MODE_LINEAR,// VkSamplerMipmapMode mipmapMode
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,// VkSamplerAddressMode addressModeU
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,// VkSamplerAddressMode addressModeV
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,// VkSamplerAddressMode addressModeW
			0.0f,// float mipLodBias
			VK_FALSE,// VkBool32 anisotropyEnable
			1.0f,// float maxAnisotropy
			VK_FALSE,// VkBool32 compareEnable
			VK_COMPARE_OP_NEVER,// VkCompareOp compareOp
			0.0f,// float minLod
			1.0f,// float maxLod
			VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,// VkBorderColor borderColor
			VK_FALSE// VkBool32 unnormalizedCoordinates
		};
		
		MemoryUsage memoryUsage = MEMORY_USAGE_GPU_ONLY;
		
		void SetAccessQueues(const std::vector<uint32_t>& queues);
		
		virtual void Create(Device* device);
		virtual void Destroy();
		
		void TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void TransitionImageLayout(Device* device, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, VkImageAspectFlags aspectMask = 0);
		
	};
	
}
