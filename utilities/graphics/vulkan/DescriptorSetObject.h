#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/BufferObject.h"
#include "utilities/graphics/FrameBufferedObject.hpp"

namespace v4d::graphics::vulkan {

class V4DLIB DescriptorSetObject {
	COMMON_OBJECT(DescriptorSetObject, VkDescriptorSet, V4DLIB)
	
	Device* device = nullptr;
	bool isDirty = false;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	
	struct V4DLIB Descriptor {
		VkShaderStageFlags stageFlags;
		uint32_t count;
		
		Descriptor(VkShaderStageFlags stageFlags, uint32_t count)
		 : stageFlags(stageFlags), count(count) {}
		
		virtual ~Descriptor();
		
		virtual const VkSampler* GetImmutableSamplersPtr() const {return nullptr;}
		virtual constexpr VkDescriptorType GetDescriptorType() const = 0;
		virtual void FillWriteInfo(VkWriteDescriptorSet&) = 0;
		// VkWriteDescriptorSet properties to be written via FillWriteInfo():
			// const void*                      pNext;
			// uint32_t                         dstArrayElement;
			// const VkDescriptorImageInfo*     pImageInfo;
			// const VkDescriptorBufferInfo*    pBufferInfo;
			// const VkBufferView*              pTexelBufferView;
	};
	
	// Unimplemented yet:
		// VK_DESCRIPTOR_TYPE_SAMPLER
		// VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		// VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
	
	std::map<uint32_t/*binding*/, std::unique_ptr<Descriptor>> descriptorBindings {};
	
	template<class DescriptorType, class ObjectType>
	void SetBinding(uint32_t binding, const ObjectType& obj, VkShaderStageFlags stageFlags, uint32_t count = 1) {
		descriptorBindings[binding] = std::make_unique<DescriptorType>(obj, stageFlags, count);
		isDirty = true;
	}
	
	void CreateDescriptorSetLayout(Device* device);
	void DestroyDescriptorSetLayout();
	
	void Allocate(VkDescriptorPool);
	void Free(VkDescriptorPool);
	
	std::vector<VkWriteDescriptorSet> GetUpdateWrites();
	
};

class V4DLIB FrameBufferedDescriptorSetObject {
	std::array<DescriptorSetObject, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES> set;
	
public:
	
	template<class DescriptorType, class ObjectType>
	inline void SetBinding(uint32_t binding, const ObjectType& obj, VkShaderStageFlags stageFlags, uint32_t count = 1) {
		for (auto& s : set) s.SetBinding<DescriptorType>(binding, obj, stageFlags, count);
	}
	
	template<class DescriptorType, class ObjectType>
	inline void SetBinding(uint32_t binding, const FrameBufferedObject<ObjectType>& obj, VkShaderStageFlags stageFlags, uint32_t count = 1) {
		for (size_t i = 0; i < set.size(); ++i) set[i].SetBinding<DescriptorType>(binding, obj[i], stageFlags, count);
	}
	
	template<class DescriptorType>
	inline void SetBinding(uint32_t binding, const FrameBuffered_ImageObject& obj, VkShaderStageFlags stageFlags, uint32_t count = 1) {
		for (size_t i = 0; i < set.size(); ++i) set[i].SetBinding<DescriptorType, ImageObject>(binding, obj[i], stageFlags, count);
	}
	
	inline DescriptorSetObject& operator[](size_t frameIndex) {
		assert(frameIndex < set.size());
		return set[frameIndex];
	}
	
};

#pragma region Descriptor types

	struct StorageBufferDescriptor : DescriptorSetObject::Descriptor {
		const VkBuffer* bufferPtr;
		VkDeviceSize bufferSize;
		VkDeviceSize bufferOffset;
		VkDescriptorBufferInfo info {};
		
		StorageBufferDescriptor(const StorageBufferDescriptor&) = default;
		StorageBufferDescriptor(StorageBufferDescriptor&&) = default;
		
		StorageBufferDescriptor(const VkBuffer* bufferPtr, VkDeviceSize bufferSize, VkDeviceSize bufferOffset = 0, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL, uint32_t count = 1)
			: Descriptor(stageFlags, count), bufferPtr(bufferPtr), bufferSize(bufferSize), bufferOffset(bufferOffset) {}
			
		StorageBufferDescriptor(const BufferObject& buffer, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL, uint32_t count = 1)
			: StorageBufferDescriptor(buffer.obj, buffer.size, 0, stageFlags, count) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			assert(bufferPtr);
			info.buffer = *bufferPtr;
			info.offset = bufferOffset;
			info.range = bufferSize;
			write.pBufferInfo = &info;
		}
	};

	struct UniformBufferDescriptor : DescriptorSetObject::Descriptor {
		const VkBuffer* bufferPtr;
		VkDeviceSize bufferSize;
		VkDeviceSize bufferOffset;
		VkDescriptorBufferInfo info {};
		
		UniformBufferDescriptor(const UniformBufferDescriptor&) = default;
		UniformBufferDescriptor(UniformBufferDescriptor&&) = default;
		
		UniformBufferDescriptor(const VkBuffer* bufferPtr, VkDeviceSize bufferSize, VkDeviceSize bufferOffset = 0, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL, uint32_t count = 1)
			: Descriptor(stageFlags, count), bufferPtr(bufferPtr), bufferSize(bufferSize), bufferOffset(bufferOffset) {}
			
		UniformBufferDescriptor(const BufferObject& buffer, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL, uint32_t count = 1)
			: UniformBufferDescriptor(buffer.obj, buffer.size, 0, stageFlags, count) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			assert(bufferPtr);
			info.buffer = *bufferPtr;
			info.offset = bufferOffset;
			info.range = bufferSize;
			write.pBufferInfo = &info;
		}
	};

	struct StorageImageDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		VkDescriptorImageInfo info {};
		
		StorageImageDescriptor(const StorageImageDescriptor&) = default;
		StorageImageDescriptor(StorageImageDescriptor&&) = default;
		
		StorageImageDescriptor(const VkImageView* imageView, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, uint32_t count = 1)
			: Descriptor(stageFlags, count), imageViewPtr(imageView) {}
			
		StorageImageDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, uint32_t count = 1)
			: StorageImageDescriptor(&image.view, stageFlags, count) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			assert(imageViewPtr);
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			info.imageView = *imageViewPtr;
			write.pImageInfo = &info;
		}
	};

	struct CombinedImageSamplerDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		const VkSampler* samplerPtr;
		VkDescriptorImageInfo info {};
		
		CombinedImageSamplerDescriptor(const CombinedImageSamplerDescriptor&) = default;
		CombinedImageSamplerDescriptor(CombinedImageSamplerDescriptor&&) = default;
		
		CombinedImageSamplerDescriptor(const VkImageView* imageView, const VkSampler* sampler, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, uint32_t count = 1)
			: Descriptor(stageFlags, count), imageViewPtr(imageView), samplerPtr(sampler) {}
			
		CombinedImageSamplerDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, uint32_t count = 1)
			: CombinedImageSamplerDescriptor(&image.view, &image.sampler, stageFlags, count) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			assert(imageViewPtr);
			assert(samplerPtr);
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			info.sampler = *samplerPtr;
			info.imageView = *imageViewPtr;
			write.pImageInfo = &info;
		}
	};

	struct InputAttachmentDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo info {};
		
		InputAttachmentDescriptor(const InputAttachmentDescriptor&) = default;
		InputAttachmentDescriptor(InputAttachmentDescriptor&&) = default;
		
		InputAttachmentDescriptor(const VkImageView* imageView, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, uint32_t count = 1)
			: Descriptor(stageFlags, count), imageViewPtr(imageView) {}
			
		InputAttachmentDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, uint32_t count = 1)
			: InputAttachmentDescriptor(&image.view, stageFlags, count) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			assert(imageViewPtr);
			info.imageLayout = imageLayout;
			info.imageView = *imageViewPtr;
			write.pImageInfo = &info;
		}
	};
	
	struct AccelerationStructureDescriptor : DescriptorSetObject::Descriptor {
		const VkAccelerationStructureKHR* tlas;
		VkWriteDescriptorSetAccelerationStructureKHR info {};
		
		AccelerationStructureDescriptor(const AccelerationStructureDescriptor&) = default;
		AccelerationStructureDescriptor(AccelerationStructureDescriptor&&) = default;
		
		AccelerationStructureDescriptor(const VkAccelerationStructureKHR* tlas, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR, uint32_t count = 1)
			: Descriptor(stageFlags, count), tlas(tlas) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			info.accelerationStructureCount = count;
			info.pAccelerationStructures = tlas;
			write.pNext = &info;
		}
	};

#pragma endregion

}
