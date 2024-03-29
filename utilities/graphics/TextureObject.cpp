#define STB_IMAGE_IMPLEMENTATION
#include "TextureObject.h"
#include "vulkan/Instance.h"
#include "vulkan/ImageObject.h"

namespace v4d::graphics {
COMMON_OBJECT_CPP(TextureObject, VkImage)

	TextureObject::TextureObject(uint32_t width, uint32_t height, int componentCount)
	: obj(), width(width), height(height), componentCount(componentCount), data(nullptr), bufferSize(0), keepStagingBufferAlive(true) {
		imageInfo.mipLevels = 1;
		bufferSize = width * height * componentCount;
	}
	
	TextureObject::TextureObject(const char* filepath, uint32_t mipLevels) : obj() {
		imageInfo.mipLevels = mipLevels;
		Load(filepath);
		bufferSize = width * height * componentCount;
	}
	
	TextureObject::TextureObject(uint32_t width, uint32_t height, int componentCount, byte* data, size_t bufferSize)
	 : obj(), width(width), height(height), componentCount(componentCount), data(data), bufferSize(bufferSize) {}
	
	TextureObject::~TextureObject() {
		if (ownedData) stbi_image_free(ownedData);
	}
	
	void TextureObject::Load(const char* filepath) {
		std::lock_guard lock(mu);
		if (ownedData) stbi_image_free(ownedData);
		ownedData = stbi_load(filepath, &width, &height, &componentCount, STBI_default);
		
		if (componentCount == 3) {
			ownedData = stbi__convert_format(ownedData, 3, 4, width, height);
			componentCount = 4;
		}
		
		if (!ownedData){
			throw std::runtime_error("Failed to load texture '" + std::string(filepath) + "' : " + stbi_failure_reason());
		}
		
		data = ownedData;
		dirty = true;
	}
	
	void TextureObject::Load(const byte* fileContent, size_t fileSize) {
		std::lock_guard lock(mu);
		if (ownedData) stbi_image_free(ownedData);
		ownedData = stbi_load_from_memory(fileContent, fileSize, &width, &height, &componentCount, STBI_default);
		
		if (componentCount == 3) {
			ownedData = stbi__convert_format(ownedData, 3, 4, width, height);
			componentCount = 4;
		}
		
		if (!ownedData){
			throw std::runtime_error(std::string("Failed to load dynamic texture: ") + stbi_failure_reason());
		}
		
		data = ownedData;
		dirty = true;
	}
	
	void TextureObject::Create(Device* device) {
		if (this->device == nullptr || dirty) {
			this->device = device;
			dirty = false;
			
			VkBufferCreateInfo bufferInfo {};{
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = bufferSize;
				bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bufferInfo.queueFamilyIndexCount = 0;
				bufferInfo.pQueueFamilyIndices = nullptr;
			}
			
			if (!stagingBuffer) {
				device->CreateAndAllocateBuffer(bufferInfo, MEMORY_USAGE_CPU_ONLY, stagingBuffer, &stagingBufferAllocation);
			}
			
			if (VkImage(obj) == VK_NULL_HANDLE) {
				imageInfo.extent.width = uint32_t(width);
				imageInfo.extent.height = uint32_t(height);
				imageInfo.mipLevels = glm::min(imageInfo.mipLevels, uint32_t(glm::floor(glm::log2(glm::max(imageInfo.extent.width, imageInfo.extent.height, imageInfo.extent.depth))))+1);
				switch (componentCount) {
					case STBI_grey:
						imageInfo.format = VK_FORMAT_R8_UNORM;
						break;
					case STBI_grey_alpha:
						imageInfo.format = VK_FORMAT_R8G8_UNORM;
						break;
					case STBI_rgb: // RGB format is unsupported in Vulkan, we need to convert it to RGBA (done in Load())...
						imageInfo.format = VK_FORMAT_R8G8B8_UNORM;
						break;
					case STBI_rgb_alpha: default:
						imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
						break;
				}
				Instance::CheckVkResult("Create and allocate image", device->CreateAndAllocateImage(imageInfo, memoryUsage, obj, &allocation));
			} else {
				assert(imageInfo.extent.width == uint32_t(width) && imageInfo.extent.height == uint32_t(height));
			}
			
			{// Copy image data to staging buffer
				std::lock_guard lock(mu);
				if (data) {
					void* stagingData;
					device->MapMemoryAllocation(stagingBufferAllocation, &stagingData, 0, bufferSize);
					memcpy(stagingData, data, bufferSize);
					device->UnmapMemoryAllocation(stagingBufferAllocation);
				}
			}
			
			// Copy staging buffer to image and generate mip maps
			device->RunSingleTimeCommands(device->queues[0][0], [&](VkCommandBuffer cmdBuffer){
				ImageObject::TransitionImageLayout(device, cmdBuffer, obj, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageInfo.mipLevels, imageInfo.arrayLayers);
				
				VkBufferImageCopy region = {};
					region.bufferOffset = 0;
					region.bufferRowLength = 0;
					region.bufferImageHeight = 0;
					region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					region.imageSubresource.mipLevel = 0;
					region.imageSubresource.baseArrayLayer = 0;
					region.imageSubresource.layerCount = 1;
					region.imageOffset = {0, 0, 0};
					region.imageExtent = imageInfo.extent;

				device->CmdCopyBufferToImage(
					cmdBuffer,
					stagingBuffer,
					obj,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
				
				if (imageInfo.mipLevels > 1) GenerateMipmaps(cmdBuffer);
				
				ImageObject::TransitionImageLayout(device, cmdBuffer, obj, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, imageInfo.mipLevels, imageInfo.arrayLayers);
			});
			
			if (!keepStagingBufferAlive) {
				// Destroy staging buffer
				device->FreeAndDestroyBuffer(stagingBuffer, stagingBufferAllocation);
			}
		}
	}

	void TextureObject::Destroy() {
		if (device) {
			if (VkImage(obj) != VK_NULL_HANDLE) {
				device->FreeAndDestroyImage(obj, allocation);
			}
			if (stagingBuffer != VK_NULL_HANDLE) {
				device->FreeAndDestroyBuffer(stagingBuffer, stagingBufferAllocation);
			}
			device = nullptr;
		}
	}

	void TextureObject::GenerateMipmaps(VkCommandBuffer commandBuffer) {
		{
			VkFormatProperties formatProperties;
			device->GetPhysicalDevice()->GetPhysicalDeviceFormatProperties(imageInfo.format, &formatProperties);
			if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				throw std::runtime_error("Texture image format does not support linear blitting");
			}
		}

		VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = obj;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < imageInfo.mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			device->CmdPipelineBarrier(
				commandBuffer, 
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit = {};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth>1? mipWidth/2 : 1, mipHeight>1? mipHeight/2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

			device->CmdBlitImage(
				commandBuffer,
				obj, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				obj, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.levelCount = imageInfo.mipLevels - 1;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		device->CmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}
	
}
