#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	struct V4DLIB PipelineLayout {
		
		std::vector<DescriptorSet*> descriptorSets {};
		std::vector<VkDescriptorSetLayout> layouts {};
		std::vector<VkDescriptorSet> vkDescriptorSets {};
		VkPipelineLayout handle = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts();

		void AddDescriptorSet(DescriptorSet* descriptorSet);
		
		void Create(Device* device);
		void Destroy(Device* device);
		
		void Bind(Device* device, VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		
	};
}
