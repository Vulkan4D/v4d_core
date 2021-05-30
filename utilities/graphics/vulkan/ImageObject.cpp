#include "ImageObject.h"

namespace v4d::graphics::vulkan {
COMMON_OBJECT_CPP(ImageObject, VkImage)

ImageObject::ImageObject(VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers, const std::vector<VkFormat>& preferredFormats, VkImageViewType viewType)
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
	viewInfo.viewType = viewType;
}

ImageObject::~ImageObject() {}

void ImageObject::Create(Device* device) {
	if (this->device == nullptr) {
		this->device = device;
		
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		
		// Map usage to features
		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) formatFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) formatFeatures |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) formatFeatures |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT) formatFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
		if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) formatFeatures |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
		if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) formatFeatures |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
		
		format = device->GetPhysicalDevice()->FindSupportedFormat(preferredFormats, imageInfo.tiling, (VkFormatFeatureFlagBits)formatFeatures);
		imageInfo.format = format;
		viewInfo.format = format;
		
		if (viewInfo.viewType == VK_IMAGE_VIEW_TYPE_CUBE) imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		
		if (sharedAccessQueues.size() > 2 || (sharedAccessQueues.size() == 2 && sharedAccessQueues[0] != sharedAccessQueues[1])) {
			imageInfo.queueFamilyIndexCount = sharedAccessQueues.size();
			imageInfo.pQueueFamilyIndices = sharedAccessQueues.data();
			imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		} else {
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		
		device->CreateAndAllocateImage(imageInfo, memoryUsage, obj, &allocation);
		
		if (format == VK_FORMAT_D32_SFLOAT) {
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		
		viewInfo.image = obj;
		if (device->CreateImageView(&viewInfo, nullptr, &view) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
			
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
			if (device->CreateSampler(&samplerInfo, nullptr, &sampler) != VK_SUCCESS)
				throw std::runtime_error("Failed to create sampler");
		}
	}
}

void ImageObject::Destroy() {
	if (device) {
		if (sampler != VK_NULL_HANDLE) {
			device->DestroySampler(sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}
		if (view != VK_NULL_HANDLE) {
			device->DestroyImageView(view, nullptr);
			view = VK_NULL_HANDLE;
		}
		if (VkImage(obj) != VK_NULL_HANDLE) {
			device->FreeAndDestroyImage(obj, allocation);
		}
		device = nullptr;
		width = 0;
		height = 0;
	}
}


void ImageObject::TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
	assert(device);
	VkImageAspectFlags aspectMask = 0;
	if (format == VK_FORMAT_D32_SFLOAT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (format == VK_FORMAT_D32_SFLOAT_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if (!aspectMask && (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	TransitionImageLayout(device, commandBuffer, obj, oldLayout, newLayout, mipLevels, arrayLayers, aspectMask);
}

void ImageObject::TransitionImageLayout(Device* device, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, VkImageAspectFlags aspectMask) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout; // VK_IMAGE_LAYOUT_UNDEFINED if we dont care about existing contents of the image
	barrier.newLayout = newLayout;
	// If we are using the barrier to transfer queue family ownership, these two fields should be the indices of the queue families. Otherwise VK_QUEUE_FAMILY_IGNORED
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	//
	if (aspectMask) {
		barrier.subresourceRange.aspectMask = aspectMask;
	} else {
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		} else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	//
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	
	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source 
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	/*
	Transfer writes must occur in the pipeline transfer stage. 
	Since the writes dont have to wait on anything, we mayy specify an empty access mask and the earliest possible pipeline stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT for the pre-barrier operations.
	It should be noted that VK_PIPELINE_STAGE_TRANSFER_BIT is not a Real stage within the graphics and compute pipelines.
	It is more of a pseudo-stage where transfers happen.
	
	The image will be written in the same pipeline stage and subsequently read by the fragment shader, which is why we specify shader reading access in the fragment pipeline stage.
	If we need to do more transitions in the future, then we'll extend the function.
	
	One thing to note is that command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT synchronization at the beginning.
	Since the TransitionImageLayout function executes a command buffer with only a single command, we could use this implicit synchronization and set srcAccessMask to 0 if we ever needed a VK_ACCESS_HOST_WRITE_BIT dependency in a layout transition.

	There is actually a special type of image layout that supports all operations, VK_IMAGE_LAYOUT_GENERAL.
	The problem with it is that it doesnt necessarily offer the best performance for any operation.
	It is required for some special cases, like using an image as both input and output, or for reading an image after it has left the preinitialized layout.
	*/
	//
	device->CmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

}
