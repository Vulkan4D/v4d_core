#pragma once
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#include "stb/stb_image.h"
#include <v4d.h>
#include "vulkan/Device.h"

namespace v4d::graphics {
using namespace vulkan;
struct V4DLIB TextureObject {
COMMON_OBJECT(TextureObject, VkImage, V4DLIB)

	int width = 0;
	int height = 0;
	int componentCount = 0;
	byte* data = nullptr;
	byte* ownedData = nullptr;
	size_t bufferSize = 0;
	
	MemoryUsage memoryUsage = MEMORY_USAGE_GPU_ONLY;
	MemoryAllocation allocation = VK_NULL_HANDLE;
	Device* device = nullptr;
	
	VkImageCreateInfo imageInfo {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,// const void* pNext
		0,// VkImageCreateFlags flags
		VK_IMAGE_TYPE_2D,// VkImageType imageType
		VK_FORMAT_R32G32B32A32_SFLOAT,// VkFormat format
		{0,0,1},// VkExtent3D extent
		8,// uint32_t mipLevels
		1,// uint32_t arrayLayers
		VK_SAMPLE_COUNT_1_BIT,// VkSampleCountFlagBits samples
		VK_IMAGE_TILING_OPTIMAL,// VkImageTiling tiling
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,// VkImageUsageFlags usage
		VK_SHARING_MODE_EXCLUSIVE,// VkSharingMode sharingMode
		0,// uint32_t queueFamilyIndexCount
		nullptr,// const uint32_t* pQueueFamilyIndices
		VK_IMAGE_LAYOUT_UNDEFINED// VkImageLayout initialLayout
	};
	
	TextureObject(const char* filepath);
	TextureObject(uint32_t width, uint32_t height, int componentCount, byte* data, size_t bufferSize);
	
	~TextureObject();
	
	void Create(Device* device);
	void Destroy();
	
	void GenerateMipmaps(VkCommandBuffer);
	
};
}
