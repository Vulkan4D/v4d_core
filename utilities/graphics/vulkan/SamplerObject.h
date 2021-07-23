#pragma once
#include "../TextureObject.h"
#include "ImageObject.h"
#include <v4d.h>

namespace v4d::graphics::vulkan {
using namespace v4d::graphics;
struct V4DLIB SamplerObject {
COMMON_OBJECT(SamplerObject, VkSampler, V4DLIB)
	VkImageView view = VK_NULL_HANDLE;
	std::shared_ptr<TextureObject> texture = nullptr;
	
	SamplerObject(std::shared_ptr<TextureObject> texture, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addrModeU, VkSamplerAddressMode addrModeV, VkSamplerAddressMode addrModeW)
	 : obj(), view(), texture(texture)
	{
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = addrModeU;
		samplerInfo.addressModeV = addrModeV;
		samplerInfo.addressModeW = addrModeW;
	}

	SamplerObject(ImageObject* image)
	 : obj(image->sampler), view(image->view), texture(nullptr), viewInfo(image->viewInfo), samplerInfo(image->samplerInfo)
	{}

	VkImageViewCreateInfo viewInfo {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,// VkStructureType sType
		nullptr,// const void* pNext
		0,// VkImageViewCreateFlags flags
		VK_NULL_HANDLE,// VkImage image
		VK_IMAGE_VIEW_TYPE_2D,// VkImageViewType viewType
		VK_FORMAT_R8G8B8A8_UNORM,// VkFormat format
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
		-1.0f,// float mipLodBias
		VK_TRUE,// VkBool32 anisotropyEnable
		8.0f,// float maxAnisotropy
		VK_FALSE,// VkBool32 compareEnable
		VK_COMPARE_OP_NEVER,// VkCompareOp compareOp
		0.0f,// float minLod
		VK_LOD_CLAMP_NONE,// float maxLod
		VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,// VkBorderColor borderColor
		VK_FALSE// VkBool32 unnormalizedCoordinates
	};
	
	void Create();
	void Destroy();
	
};
}
