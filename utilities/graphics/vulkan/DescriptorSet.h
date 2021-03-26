/*
 * Vulkan DescriptorSet abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This weird class is a work in progress
 */
#pragma once

#include <v4d.h>
#include <vector>
#include <map>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Buffer.h"
#include "utilities/graphics/vulkan/Image.h"

namespace v4d::graphics::vulkan {
	
	enum DescriptorPointerType {
		STORAGE_BUFFER, // BufferDescriptor ---> VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		UNIFORM_BUFFER, // BufferDescriptor ---> VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		IMAGE_VIEW, // VkImageView ---> VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		ACCELERATION_STRUCTURE, // VkAccelerationStructureKHR ---> VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
		COMBINED_IMAGE_SAMPLER, // VkImageView ---> VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		INPUT_ATTACHMENT, // VkImageView ---> VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		INPUT_ATTACHMENT_DEPTH_STENCIL, // VkImageView ---> VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
	};

	#define __V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE(descriptor_type, data_ptr_type, name, stage_flags, t)\
		void AddBinding_ ## name ( \
			uint32_t binding, \
			data_ptr_type* data, \
			VkShaderStageFlags stageFlags = stage_flags, \
			VkDescriptorType type = t, \
			VkSampler* pImmutableSamplers = nullptr \
		) { \
			bindings[binding] = { \
				binding, \
				stageFlags, \
				type, \
				1, \
				0, \
				descriptor_type, \
				pImmutableSamplers, \
				data, \
				nullptr \
			}; \
		}\
		void AddBinding_ ## name ## _array ( \
			uint32_t binding, \
			data_ptr_type* data, \
			uint32_t count, \
			VkShaderStageFlags stageFlags = stage_flags, \
			VkDescriptorType type = t, \
			VkSampler* pImmutableSamplers = nullptr \
		) { \
			bindings[binding] = { \
				binding, \
				stageFlags, \
				type, \
				count, \
				0, \
				descriptor_type, \
				pImmutableSamplers, \
				data, \
				nullptr \
			}; \
		}
		
	#define __V4D_DESCRIPTOR_SET_DEFINE_BINDINGS\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( STORAGE_BUFFER, BufferDescriptor, storageBuffer, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( UNIFORM_BUFFER, BufferDescriptor, uniformBuffer, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( IMAGE_VIEW, Image, imageView, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( COMBINED_IMAGE_SAMPLER, Image, combinedImageSampler, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( ACCELERATION_STRUCTURE, VkAccelerationStructureKHR, accelerationStructure, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( INPUT_ATTACHMENT, Image, inputAttachment, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT )\
		__V4D_DESCRIPTOR_SET_ADD_BINDING_TYPE( INPUT_ATTACHMENT_DEPTH_STENCIL, Image, inputAttachmentDepthStencil, VK_SHADER_STAGE_ALL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT )\

	// Not implemented yet
		// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		// VK_DESCRIPTOR_TYPE_SAMPLER
		// VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		// VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		// VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
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
		bool IsWriteDescriptorSetValid() const;
	};

	class V4DLIB DescriptorSet {
	public:
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	private:
		std::map<uint32_t, DescriptorBinding> bindings {};
		
	public:
		
		DescriptorSet();
		
		std::map<uint32_t, DescriptorBinding>& GetBindings();
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
		
		void CreateDescriptorSetLayout(Device* device);
		void DestroyDescriptorSetLayout(Device* device);
		
		__V4D_DESCRIPTOR_SET_DEFINE_BINDINGS
	};
}
