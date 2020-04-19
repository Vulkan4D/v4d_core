/*
 * Vulkan Pipeline Layout abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB PipelineLayout {
		
		std::vector<DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSetLayout> layouts {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		std::vector<VkPushConstantRange> pushConstants {};

		VkPipelineLayout handle = VK_NULL_HANDLE;
		
		std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts();

		void Create(Device* device);
		void Destroy(Device* device);
		
		void AddDescriptorSet(DescriptorSet* descriptorSet);
		
		int AddPushConstant(const VkPushConstantRange&);
		template<class T>
		int AddPushConstant(VkShaderStageFlags flags) {
			return AddPushConstant({flags, 0, sizeof(T)});
		}
		
		// clears descriptor sets and push constants
		void Reset();
		
		void Bind(Device* device, VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		
	};
}
