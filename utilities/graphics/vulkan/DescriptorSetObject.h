#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/ImageObject.h"
#include "utilities/graphics/vulkan/SamplerObject.h"
#include "utilities/graphics/vulkan/BufferObject.h"
#include "utilities/graphics/FramebufferedObject.hpp"
#include "utilities/graphics/vulkan/raytracing/AccelerationStructure.h"

namespace v4d::graphics::vulkan {

class V4DLIB DescriptorSetObject {
	COMMON_OBJECT(DescriptorSetObject, VkDescriptorSet, V4DLIB)
	
	Device* device = nullptr;
	bool isDirty = false;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	
	VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsInfo {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		nullptr,// const void*                        pNext;
		0,		// uint32_t                           bindingCount;
		nullptr	// const VkDescriptorBindingFlags*    pBindingFlags;
	};
	
	uint32_t variableCount = 0;
	VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
		nullptr,// const void*        pNext;
		1,		// uint32_t           descriptorSetCount;
		&variableCount	// const uint32_t*    pDescriptorCounts;
	};
	
	struct V4DLIB Descriptor {
		VkShaderStageFlags stageFlags;
		uint32_t count;
		VkDescriptorBindingFlags bindingFlags;
		
		Descriptor(VkShaderStageFlags stageFlags, uint32_t count, bool useDescriptorIndexing = false)
		 : stageFlags(stageFlags), count(count), bindingFlags(useDescriptorIndexing? (VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) : 0) {}
		
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
	
	std::map<uint32_t/*binding*/, std::unique_ptr<Descriptor>> descriptorBindings {};
	
	template<class DescriptorType, class ObjectType>
	void SetBinding(uint32_t binding, const ObjectType& obj, VkShaderStageFlags stageFlags) {
		descriptorBindings[binding] = std::make_unique<DescriptorType>(obj, stageFlags);
		isDirty = true;
	}
	
	template<class DescriptorType>
	void SetBindingArray(uint32_t binding, uint32_t count, VkShaderStageFlags stageFlags) {
		descriptorBindings[binding] = std::make_unique<DescriptorType>(count, stageFlags);
		variableCount = count;
		isDirty = true;
	}
	
	void CreateDescriptorSetLayout(Device* device);
	void DestroyDescriptorSetLayout();
	
	void Allocate(VkDescriptorPool);
	void Free(VkDescriptorPool);
	
	std::vector<VkWriteDescriptorSet> GetUpdateWrites();
	VkWriteDescriptorSet GetUpdateWrite(uint32_t binding);
	void Update(uint32_t binding);
	
};

class V4DLIB Framebuffered_DescriptorSetObject {
	std::array<DescriptorSetObject, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES> set;
	
public:
	
	template<class DescriptorType, class ObjectType>
	inline void SetBinding(uint32_t binding, const ObjectType& obj, VkShaderStageFlags stageFlags) {
		for (auto& s : set) s.SetBinding<DescriptorType>(binding, obj, stageFlags);
	}
	
	template<class DescriptorType>
	inline void SetBindingArray(uint32_t binding, uint32_t count, VkShaderStageFlags stageFlags) {
		for (auto& s : set) s.SetBindingArray<DescriptorType>(binding, count, stageFlags);
	}
	
	template<class DescriptorType, class ObjectType>
	inline void SetBinding(uint32_t binding, const FramebufferedObject<ObjectType>& obj, VkShaderStageFlags stageFlags) {
		for (size_t i = 0; i < set.size(); ++i) set[i].SetBinding<DescriptorType>(binding, obj[i], stageFlags);
	}
	
	template<class DescriptorType>
	inline void SetBinding(uint32_t binding, const Framebuffered_ImageObject& obj, VkShaderStageFlags stageFlags) {
		for (size_t i = 0; i < set.size(); ++i) set[i].SetBinding<DescriptorType, ImageObject>(binding, obj[i], stageFlags);
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
		
		StorageBufferDescriptor(const VkBuffer* bufferPtr, VkDeviceSize bufferSize, VkDeviceSize bufferOffset = 0, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: Descriptor(stageFlags, 1), bufferPtr(bufferPtr), bufferSize(bufferSize), bufferOffset(bufferOffset) {}
		
		explicit StorageBufferDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: Descriptor(stageFlags, count, true), bufferPtr(nullptr), bufferSize(0), bufferOffset(0) {}
			
		StorageBufferDescriptor(const BufferObject& buffer, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: StorageBufferDescriptor(buffer.obj, buffer.size, 0, stageFlags) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (bufferPtr) {
				info.buffer = *bufferPtr;
				info.offset = bufferOffset;
				info.range = bufferSize;
				write.pBufferInfo = &info;
			} else write.descriptorCount = 0;
		}
	};

	struct UniformBufferDescriptor : DescriptorSetObject::Descriptor {
		const VkBuffer* bufferPtr;
		VkDeviceSize bufferSize;
		VkDeviceSize bufferOffset;
		VkDescriptorBufferInfo info {};
		
		UniformBufferDescriptor(const UniformBufferDescriptor&) = default;
		UniformBufferDescriptor(UniformBufferDescriptor&&) = default;
		
		UniformBufferDescriptor(const VkBuffer* bufferPtr, VkDeviceSize bufferSize, VkDeviceSize bufferOffset = 0, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: Descriptor(stageFlags, 1), bufferPtr(bufferPtr), bufferSize(bufferSize), bufferOffset(bufferOffset) {}
		
		explicit UniformBufferDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: Descriptor(stageFlags, count, true), bufferPtr(nullptr), bufferSize(0), bufferOffset(0) {}
			
		UniformBufferDescriptor(const BufferObject& buffer, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL)
			: UniformBufferDescriptor(buffer.obj, buffer.size, 0, stageFlags) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (bufferPtr) {
				info.buffer = *bufferPtr;
				info.offset = bufferOffset;
				info.range = bufferSize;
				write.pBufferInfo = &info;
			} else write.descriptorCount = 0;
		}
	};

	struct StorageImageDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		VkDescriptorImageInfo info {};
		
		StorageImageDescriptor(const StorageImageDescriptor&) = default;
		StorageImageDescriptor(StorageImageDescriptor&&) = default;
		
		StorageImageDescriptor(const VkImageView* imageView, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, 1), imageViewPtr(imageView) {}
			
		explicit StorageImageDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, count, true), imageViewPtr(nullptr) {}
			
		StorageImageDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: StorageImageDescriptor(&image.view, stageFlags) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (imageViewPtr) {
				info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				info.imageView = *imageViewPtr;
				write.pImageInfo = &info;
			} else write.descriptorCount = 0;
		}
	};

	struct CombinedImageSamplerDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		const VkSampler* samplerPtr;
		VkDescriptorImageInfo info {};
		
		CombinedImageSamplerDescriptor(const CombinedImageSamplerDescriptor&) = default;
		CombinedImageSamplerDescriptor(CombinedImageSamplerDescriptor&&) = default;
		
		CombinedImageSamplerDescriptor(const VkImageView* imageView, const VkSampler* sampler, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, 1), imageViewPtr(imageView), samplerPtr(sampler) {}
			
		explicit CombinedImageSamplerDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, count, true), imageViewPtr(nullptr), samplerPtr(nullptr) {}
			
		CombinedImageSamplerDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: CombinedImageSamplerDescriptor(&image.view, &image.sampler, stageFlags) {}
			
		CombinedImageSamplerDescriptor(const SamplerObject& sampler, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: CombinedImageSamplerDescriptor(&sampler.view, sampler.obj, stageFlags) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (imageViewPtr && samplerPtr) {
				info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				info.sampler = *samplerPtr;
				info.imageView = *imageViewPtr;
				write.pImageInfo = &info;
			} else write.descriptorCount = 0;
		}
	};

	struct SamplerDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		const VkSampler* samplerPtr;
		VkDescriptorImageInfo info {};
		
		SamplerDescriptor(const SamplerDescriptor&) = default;
		SamplerDescriptor(SamplerDescriptor&&) = default;
		
		SamplerDescriptor(const VkImageView* imageView, const VkSampler* sampler, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, 1), imageViewPtr(imageView), samplerPtr(sampler) {}
			
		explicit SamplerDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: Descriptor(stageFlags, count, true), imageViewPtr(nullptr), samplerPtr(nullptr) {}
			
		SamplerDescriptor(const SamplerObject& sampler, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS)
			: SamplerDescriptor(&sampler.view, sampler.obj, stageFlags) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_SAMPLER;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (imageViewPtr && samplerPtr) {
				info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				info.imageView = *imageViewPtr;
				info.sampler = *samplerPtr;
				write.pImageInfo = &info;
			} else write.descriptorCount = 0;
		}
	};

	struct InputAttachmentDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkDescriptorImageInfo info {};
		
		InputAttachmentDescriptor(const InputAttachmentDescriptor&) = default;
		InputAttachmentDescriptor(InputAttachmentDescriptor&&) = default;
		
		InputAttachmentDescriptor(const VkImageView* imageView, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: Descriptor(stageFlags, 1), imageViewPtr(imageView) {}
			
		InputAttachmentDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: InputAttachmentDescriptor(&image.view, stageFlags) {}
			
		explicit InputAttachmentDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: Descriptor(stageFlags, count, true), imageViewPtr(nullptr) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (imageViewPtr) {
				info.imageLayout = imageLayout;
				info.imageView = *imageViewPtr;
				write.pImageInfo = &info;
			} else write.descriptorCount = 0;
		}
	};
	
	struct InputOutputAttachmentDescriptor : DescriptorSetObject::Descriptor {
		const VkImageView* imageViewPtr;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		VkDescriptorImageInfo info {};
		
		InputOutputAttachmentDescriptor(const InputOutputAttachmentDescriptor&) = default;
		InputOutputAttachmentDescriptor(InputOutputAttachmentDescriptor&&) = default;
		
		InputOutputAttachmentDescriptor(const VkImageView* imageView, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: Descriptor(stageFlags, 1), imageViewPtr(imageView) {}
			
		InputOutputAttachmentDescriptor(const ImageObject& image, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: InputOutputAttachmentDescriptor(&image.view, stageFlags) {}
			
		explicit InputOutputAttachmentDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT)
			: Descriptor(stageFlags, count, true), imageViewPtr(nullptr) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (imageViewPtr) {
				info.imageLayout = imageLayout;
				info.imageView = *imageViewPtr;
				write.pImageInfo = &info;
			} else write.descriptorCount = 0;
		}
	};
	
	struct AccelerationStructureDescriptor : DescriptorSetObject::Descriptor {
		const VkAccelerationStructureKHR* tlas;
		VkWriteDescriptorSetAccelerationStructureKHR info {};
		
		AccelerationStructureDescriptor(const AccelerationStructureDescriptor&) = default;
		AccelerationStructureDescriptor(AccelerationStructureDescriptor&&) = default;
		
		AccelerationStructureDescriptor(const VkAccelerationStructureKHR* tlas, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR)
			: Descriptor(stageFlags, 1), tlas(tlas) {}
			
		AccelerationStructureDescriptor(const raytracing::AccelerationStructure& tlas, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR)
			: AccelerationStructureDescriptor(&tlas.accelerationStructure, stageFlags) {}
			
		explicit AccelerationStructureDescriptor(uint32_t count, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR)
			: Descriptor(stageFlags, count, true), tlas(nullptr) {}
			
		constexpr VkDescriptorType GetDescriptorType() const override {return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;}
		void FillWriteInfo(VkWriteDescriptorSet& write) override {
			if (tlas) {
				info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
				info.accelerationStructureCount = 1;
				info.pAccelerationStructures = tlas;
				write.pNext = &info;
			} else write.descriptorCount = 0;
		}
	};

	// Not yet implemented:
		// VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		// VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
	
#pragma endregion

}
