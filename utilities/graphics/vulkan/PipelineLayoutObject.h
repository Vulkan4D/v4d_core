/*
 * Vulkan Pipeline Layout abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once

#include <v4d.h>
#include <vector>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
// #include "utilities/graphics/vulkan/DescriptorSet.h"

namespace v4d::graphics::vulkan {
	
	struct V4DLIB PipelineLayoutObject {
		COMMON_OBJECT(PipelineLayoutObject, VkPipelineLayout, V4DLIB)
		COMMON_OBJECT_MOVEABLE(PipelineLayoutObject)
		COMMON_OBJECT_COPYABLE(PipelineLayoutObject)
		
		// std::vector<DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSetLayout> layouts {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		std::vector<VkPushConstantRange> pushConstants {};

		Device* device = nullptr;

		void Create(Device* device);
		void Destroy();
		
		// void AddDescriptorSet(DescriptorSet* descriptorSet) {
		// 	descriptorSets.push_back(descriptorSet);
		// }

		int AddPushConstant(const VkPushConstantRange& pushConstant) {
			int newIndex = pushConstants.size();
			pushConstants.push_back(pushConstant);
			return newIndex;
		}
		template<class T>
		int AddPushConstant(VkShaderStageFlags flags) {
			return AddPushConstant({flags, 0, sizeof(T)});
		}
		
		std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts() {
			return &layouts;
		}

		void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			assert(device);
			if (vkDescriptorSets.size() > 0) 
				device->CmdBindDescriptorSets(commandBuffer, bindPoint, obj, 0, (uint)vkDescriptorSets.size(), vkDescriptorSets.data(), 0, nullptr);
		}

		void Reset() {
			// descriptorSets.clear();
			pushConstants.clear();
		}
		
	};
}
