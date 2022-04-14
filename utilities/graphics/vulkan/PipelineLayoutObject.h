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
#include "utilities/graphics/vulkan/DescriptorSetObject.h"

namespace v4d::graphics::vulkan {
	
	class V4DLIB PipelineLayoutObject {
		COMMON_OBJECT(PipelineLayoutObject, VkPipelineLayout, V4DLIB)
		COMMON_OBJECT_MOVEABLE(PipelineLayoutObject)
		COMMON_OBJECT_COPYABLE(PipelineLayoutObject)
		
		std::vector<std::tuple<DescriptorSetObject*, DescriptorSetObject*>> descriptorSets {};
		std::vector<VkDescriptorSet> rawDescriptorSets {};
		std::vector<VkPushConstantRange> pushConstants {};

		Device* device = nullptr;

		void Create(Device* device);
		void Destroy();
		
		void AddDescriptorSet(DescriptorSetObject* descriptorSet) {
			descriptorSets.emplace_back(descriptorSet, nullptr);
		}

		int AddPushConstant(const VkPushConstantRange& pushConstant) {
			int newIndex = pushConstants.size();
			pushConstants.push_back(pushConstant);
			return newIndex;
		}
		template<class T>
		int AddPushConstant(VkShaderStageFlags flags, uint32_t offset = 0) {
			return AddPushConstant({flags, offset, sizeof(T)});
		}
		
		void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			assert(device);
			if (rawDescriptorSets.size() > 0) {
				device->CmdBindDescriptorSets(commandBuffer, bindPoint, obj, 0, (uint)rawDescriptorSets.size(), rawDescriptorSets.data(), 0, nullptr);
			}
		}

		void Reset() {
			descriptorSets.clear();
			rawDescriptorSets.clear();
			pushConstants.clear();
		}
		
	};
}
