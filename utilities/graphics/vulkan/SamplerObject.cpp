#include "SamplerObject.h"
#include "Instance.h"

namespace v4d::graphics::vulkan {
COMMON_OBJECT_CPP(SamplerObject, VkSampler)

	void SamplerObject::Create() {
		if (texture && texture->device && VkSampler(obj) == VK_NULL_HANDLE) {
			viewInfo.image = texture->obj;
			viewInfo.format = texture->imageInfo.format;
			viewInfo.subresourceRange.layerCount = texture->imageInfo.arrayLayers;
			viewInfo.subresourceRange.levelCount = texture->imageInfo.mipLevels;
			samplerInfo.maxLod = texture->imageInfo.mipLevels;
			Instance::CheckVkResult("Create image view", texture->device->CreateImageView(&viewInfo, nullptr, &view));
			Instance::CheckVkResult("Create sampler", texture->device->CreateSampler(&samplerInfo, nullptr, obj));
		}
	}

	void SamplerObject::Destroy() {
		if (texture && texture->device && VkSampler(obj) != VK_NULL_HANDLE) {
			texture->device->DestroySampler(obj, nullptr);
			obj = VK_NULL_HANDLE;
			if (view != VK_NULL_HANDLE) {
				texture->device->DestroyImageView(view, nullptr);
				view = VK_NULL_HANDLE;
			}
		}
	}

}
