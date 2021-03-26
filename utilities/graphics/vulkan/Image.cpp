#include "Image.h"

using namespace v4d::graphics::vulkan;

Image::Image(VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers, const std::vector<VkFormat>& preferredFormats)
: 	usage(usage),
	mipLevels(mipLevels),
	arrayLayers(arrayLayers),
	preferredFormats(preferredFormats)
{
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.usage = usage;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.layerCount = arrayLayers;
}

Image::~Image() {}

void Image::SetAccessQueues(const std::vector<uint32_t>& queues) {
	if (queues.size() > 2 || (queues.size() == 2 && queues[0] != queues[1])) {
		imageInfo.queueFamilyIndexCount = queues.size();
		imageInfo.pQueueFamilyIndices = queues.data();
		imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	} else {
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
}

void Image::Create(Device* device, uint32_t width, uint32_t height, const std::vector<VkFormat>& tryFormats, int additionalFormatFeatures) {
	this->width = width;
	this->height = height==0? width : height;
	imageInfo.extent.width = width;
	imageInfo.extent.height = this->height;
	int formatFeatures = additionalFormatFeatures;
	
	// Map usage to features
	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) formatFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) formatFeatures |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) formatFeatures |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	if (usage & VK_IMAGE_USAGE_STORAGE_BIT) formatFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
	if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) formatFeatures |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
	if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) formatFeatures |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
		
	format = device->GetPhysicalDevice()->FindSupportedFormat(tryFormats.size() > 0 ? tryFormats : preferredFormats, imageInfo.tiling, (VkFormatFeatureFlagBits)formatFeatures);
	imageInfo.format = format;
	viewInfo.format = format;
	
	device->CreateAndAllocateImage(imageInfo, memoryUsage, image, &allocation);
	
	viewInfo.image = image;
	if (device->CreateImageView(&viewInfo, nullptr, &view) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view");
		
	if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
		if (device->CreateSampler(&samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create sampler");
	}
}

void Image::Destroy(Device* device) {
	if (sampler != VK_NULL_HANDLE) {
		device->DestroySampler(sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
	if (view != VK_NULL_HANDLE) {
		device->DestroyImageView(view, nullptr);
		view = VK_NULL_HANDLE;
	}
	if (image != VK_NULL_HANDLE) {
		device->FreeAndDestroyImage(image, allocation);
	}
}

DepthStencilImage::DepthStencilImage(VkImageUsageFlags usage, const std::vector<VkFormat>& formats)
: Image(
	usage,
	1,
	1,
	formats
) {
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
}
DepthStencilImage::~DepthStencilImage() {}

DepthImage::DepthImage(VkImageUsageFlags usage, const std::vector<VkFormat>& formats)
: Image(
	usage,
	1,
	1,
	formats
) {
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
}
DepthImage::~DepthImage() {}

CubeMapImage::CubeMapImage(VkImageUsageFlags usage, const std::vector<VkFormat>& formats)
: Image(
	usage,
	1,
	6,
	formats
) {
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
}

CubeMapImage::~CubeMapImage() {}
