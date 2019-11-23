#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB CombinedImageSampler {
		VkSampler sampler;
		VkImageView imageView;
	};

	enum DescriptorPointerType {
		STORAGE_BUFFER, // Buffer ---> VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		UNIFORM_BUFFER, // Buffer ---> VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		IMAGE_VIEW, // VkImageView ---> VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		ACCELERATION_STRUCTURE, // VkAccelerationStructureNV ---> VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV
		COMBINED_IMAGE_SAMPLER, // VkImageView ---> VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	};

	#define DESCRIPTOR_SET_ADD_BINDING_TYPE(descriptor_type, data_ptr_type, name, stage_flags, t)\
		void AddBinding_ ## name ( \
			uint32_t binding, \
			data_ptr_type* data, \
			VkShaderStageFlags stageFlags = stage_flags, \
			VkDescriptorType type = t, \
			uint32_t descriptorCount = 1, \
			uint32_t dstArrayElement = 0, \
			VkSampler* pImmutableSamplers = nullptr \
		) { \
			bindings[binding] = { \
				binding, \
				stageFlags, \
				type, \
				descriptorCount, \
				dstArrayElement, \
				descriptor_type, \
				pImmutableSamplers, \
				data, \
				nullptr \
			}; \
		}
		
	#define DESCRIPTOR_SET_DEFINE_BINDINGS\
		DESCRIPTOR_SET_ADD_BINDING_TYPE( STORAGE_BUFFER, Buffer, storageBuffer, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER )\
		DESCRIPTOR_SET_ADD_BINDING_TYPE( UNIFORM_BUFFER, Buffer, uniformBuffer, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )\
		DESCRIPTOR_SET_ADD_BINDING_TYPE( IMAGE_VIEW, VkImageView, imageView, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE )\
		DESCRIPTOR_SET_ADD_BINDING_TYPE( COMBINED_IMAGE_SAMPLER, CombinedImageSampler, combinedImageSampler, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER )\
		DESCRIPTOR_SET_ADD_BINDING_TYPE( ACCELERATION_STRUCTURE, VkAccelerationStructureNV, accelerationStructure, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV )\

	// Not implemented yet
		// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_SAMPLER
		// VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		// VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		// VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
		
	struct V4DLIB DescriptorBinding {
		uint32_t binding;
		VkShaderStageFlags stageFlags;
		VkDescriptorType descriptorType;
		uint32_t descriptorCount;
		uint32_t dstArrayElement;
		DescriptorPointerType pointerType;
		VkSampler* pImmutableSamplers;
		
		void* data = nullptr;
		void* writeInfo = nullptr;
		
		~DescriptorBinding();
		
		VkWriteDescriptorSet GetWriteDescriptorSet(VkDescriptorSet descriptorSet);
	};

	class V4DLIB DescriptorSet {
	public:
		uint32_t set;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	private:
		std::map<uint32_t, DescriptorBinding> bindings {};
		
	public:
		
		DescriptorSet(uint32_t set);
		
		std::map<uint32_t, DescriptorBinding>& GetBindings();
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
		
		void CreateDescriptorSetLayout(Device* device);
		void DestroyDescriptorSetLayout(Device* device);
		
		DESCRIPTOR_SET_DEFINE_BINDINGS
	};
}
