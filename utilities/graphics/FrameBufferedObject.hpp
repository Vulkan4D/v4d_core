#pragma once

#include <type_traits>
#include <array>
#include <utility>
#include <v4d.h>
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics {

template<class T>
struct FrameBufferedObject {
	using FRAMEBUFFERED_ARRAY = std::array<T, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES>;
	FRAMEBUFFERED_ARRAY objArray;
	const T& operator[](size_t i) const {return objArray[i];}
	T& operator[](size_t i) {return objArray[i];}
	template<typename...Args> requires std::is_constructible_v<T, Args...>
	FrameBufferedObject(Args&&...args) {
		objArray.fill({std::move(args)...});
	}
	operator const FRAMEBUFFERED_ARRAY&() const {
		return objArray;
	}
	operator FRAMEBUFFERED_ARRAY() const {
		return objArray;
	}
};

struct FrameBuffered_Image : FrameBufferedObject<vulkan::Image> {
	VkImageUsageFlags usage;
	uint32_t mipLevels;
	uint32_t arrayLayers;
	std::vector<VkFormat> preferredFormats;
	
	FrameBuffered_Image(
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		uint32_t mipLevels = 1,
		uint32_t arrayLayers = 1,
		const std::vector<VkFormat>& preferredFormats = {VK_FORMAT_R32G32B32A32_SFLOAT}
	): 	usage(usage),
		mipLevels(mipLevels),
		arrayLayers(arrayLayers),
		preferredFormats(preferredFormats)
	{}
	
	void Create(vulkan::Device* device, uint32_t width, uint32_t height = 0, const std::vector<VkFormat>& tryFormats = {}, int additionalFormatFeatures = 0) {
		for (auto&i : objArray) {
			i = {usage, mipLevels, arrayLayers, preferredFormats};
			i.Create(device, width, height, tryFormats, additionalFormatFeatures);
		}
	}
	void Destroy(vulkan::Device* device) {
		for (auto&i : objArray) i.Destroy(device);
	}
	void TransitionLayout(vulkan::Device* device, VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1) {
		for (auto&i : objArray) i.TransitionLayout(device, commandBuffer, oldLayout, newLayout, mipLevels, layerCount);
	}
	operator std::vector<const vulkan::Image*>() const {
		std::vector<const vulkan::Image*> vec {};
		for (auto&i : objArray) vec.emplace_back(&i);
		return vec;
	}
};

}
