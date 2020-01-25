/*
 * Vulkan Image abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * Included in this file is the Image class and some of it's children classes
 * These classes contains abstractions for VkImage, VkImageView and VkImageSampler
 */
#pragma once

#include <v4d.h>

namespace v4d::graphics::vulkan {
	class V4DLIB Image {
	public:
		
		// Constructor args
		VkImageUsageFlags usage;
		uint32_t mipLevels;
		uint32_t arrayLayers;
		std::vector<VkFormat> preferredFormats;
		
		// After Create()
		uint32_t width = 0;
		uint32_t height = 0;
		VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		
		Image(
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			uint32_t mipLevels = 1,
			uint32_t arrayLayers = 1,
			const std::vector<VkFormat>& preferredFormats = {VK_FORMAT_R32G32B32A32_SFLOAT}
		);
		
		virtual ~Image();
		
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
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,// VkSamplerAddressMode addressModeU
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,// VkSamplerAddressMode addressModeV
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,// VkSamplerAddressMode addressModeW
			0.0f,// float mipLodBias
			VK_TRUE,// VkBool32 anisotropyEnable
			1.0f,// float maxAnisotropy
			VK_FALSE,// VkBool32 compareEnable
			VK_COMPARE_OP_NEVER,// VkCompareOp compareOp
			0.0f,// float minLod
			1.0f,// float maxLod
			VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,// VkBorderColor borderColor
			VK_FALSE// VkBool32 unnormalizedCoordinates
		};
		VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		void SetAccessQueues(const std::vector<uint32_t>& queues);
		
		virtual void Create(Device* device, uint32_t width, uint32_t height, const std::vector<VkFormat>& tryFormats = {}, int additionalFormatFeatures = 0);
		virtual void Destroy(Device* device);
		
	};
	
	class V4DLIB DepthStencilImage : public Image {
	public:
		DepthStencilImage(
			VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			const std::vector<VkFormat>& formats = {VK_FORMAT_D32_SFLOAT_S8_UINT}
		);
		virtual ~DepthStencilImage();
	};
	
	class V4DLIB DepthImage : public Image {
	public:
		DepthImage(
			VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			const std::vector<VkFormat>& formats = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT}
		);
		virtual ~DepthImage();
	};
	
	class V4DLIB CubeMapImage : public Image {
	public:
		CubeMapImage(
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			const std::vector<VkFormat>& formats = {VK_FORMAT_R32G32B32A32_SFLOAT}
		);
		virtual ~CubeMapImage();
	};
	
}
