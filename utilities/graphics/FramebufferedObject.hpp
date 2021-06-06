#pragma once

#include <type_traits>
#include <array>
#include <utility>
#include <v4d.h>
#include "utilities/graphics/vulkan/ImageObject.h"
#include "utilities/graphics/vulkan/Device.h"

namespace v4d::graphics {

template<class T>
struct FramebufferedObject {
	using FRAMEBUFFERED_ARRAY = std::array<T, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES>;
	FRAMEBUFFERED_ARRAY objArray;
	const T& operator[](size_t i) const {return objArray[i];}
	T& operator[](size_t i) {return objArray[i];}
	template<typename...Args> requires std::is_constructible_v<T, Args...>
	FramebufferedObject(Args&&...args) {
		objArray.fill(T{std::move(args)...});
	}
	template<typename V>
	FramebufferedObject<T>& operator=(const V& value) {
		for (T& obj : objArray) {
			obj = value;
		}
		return *this;
	}
	operator const FRAMEBUFFERED_ARRAY&() const {
		return objArray;
	}
	operator FRAMEBUFFERED_ARRAY() const {
		return objArray;
	}
	void ForEachFrame(const std::function<void(int, T&)>& func) {
		for (size_t i = 0; i < V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES; ++i) {
			func(int(i), objArray[i]);
		}
	}
};

struct Framebuffered_ImageObject : FramebufferedObject<vulkan::ImageObject> {
	Framebuffered_ImageObject(
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		uint32_t mipLevels = 1,
		uint32_t arrayLayers = 1,
		const std::vector<VkFormat>& preferredFormats = {VK_FORMAT_R32G32B32A32_SFLOAT},
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D
	) : FramebufferedObject(usage, mipLevels, arrayLayers, preferredFormats, viewType) {}
	
	operator std::vector<const vulkan::ImageObject*>() const {
		std::vector<const vulkan::ImageObject*> vec {};
		for (auto&i : objArray) vec.emplace_back(&i);
		return vec;
	}
};

}
