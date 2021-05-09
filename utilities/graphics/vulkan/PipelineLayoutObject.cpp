#include "PipelineLayoutObject.h"

using namespace v4d::graphics::vulkan;

COMMON_OBJECT_CPP(PipelineLayoutObject, VkPipelineLayout)

void PipelineLayoutObject::Create(Device* device) {
	this->device = device;
	
	// for (auto* set : descriptorSets) {
	// 	layouts.push_back(set->GetDescriptorSetLayout());
	// }
	
	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
	// Descriptor sets
	if (layouts.size() > 0) {
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
	}

	// Push constants
	if (pushConstants.size() > 0) {
		pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
	}

	if (device->CreatePipelineLayout(&pipelineLayoutInfo, nullptr, obj) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
	
	// // Descriptor sets array
	// vkDescriptorSets.reserve(descriptorSets.size());
	// for (auto* set : descriptorSets) {
	// 	vkDescriptorSets.push_back(set->descriptorSet);
	// }
}

void PipelineLayoutObject::Destroy() {
	assert(device);
	vkDescriptorSets.clear();
	device->DestroyPipelineLayout(obj, nullptr);
	layouts.clear();
	device = nullptr;
}
