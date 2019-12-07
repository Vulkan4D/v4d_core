#include <v4d.h>

using namespace v4d::graphics::vulkan;

std::vector<VkDescriptorSetLayout>* PipelineLayout::GetDescriptorSetLayouts() {
	return &layouts;
}

void PipelineLayout::AddDescriptorSet(DescriptorSet* descriptorSet) {
	descriptorSets.push_back(descriptorSet);
}

void PipelineLayout::Create(Device* device) {
	for (auto* set : descriptorSets) {
		layouts.push_back(set->GetDescriptorSetLayout());
	}
	
	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (layouts.size() > 0) {
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
	}

	//TODO push constants
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (device->CreatePipelineLayout(&pipelineLayoutInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
	
	// Descriptor sets array
	vkDescriptorSets.reserve(descriptorSets.size());
	for (auto* set : descriptorSets) {
		vkDescriptorSets.push_back(set->descriptorSet);
	}
}

void PipelineLayout::Destroy(Device* device) {
	vkDescriptorSets.clear();
	device->DestroyPipelineLayout(handle, nullptr);
	layouts.clear();
}

void PipelineLayout::Bind(Device* device, VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) {
	if (vkDescriptorSets.size() > 0) 
		device->CmdBindDescriptorSets(commandBuffer, bindPoint, handle, 0, (uint)vkDescriptorSets.size(), vkDescriptorSets.data(), 0, nullptr);
}
