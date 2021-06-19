#include "PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/Instance.h"

using namespace v4d::graphics::vulkan;

COMMON_OBJECT_CPP(PipelineLayoutObject, VkPipelineLayout)

void PipelineLayoutObject::Create(Device* device) {
	assert(this->device == nullptr);
	this->device = device;
	
	std::vector<VkDescriptorSetLayout> descriptorSetsLayouts {};
	for (auto[set,framebufferedSet] : descriptorSets) {
		if (set) {
			descriptorSetsLayouts.push_back(set->layout);
		} else if (framebufferedSet) {
			descriptorSetsLayouts.push_back((*framebufferedSet)[0].layout);
		}
	}
	
	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
	// Descriptor sets
	if (descriptorSetsLayouts.size() > 0) {
		pipelineLayoutInfo.setLayoutCount = descriptorSetsLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetsLayouts.data();
	}

	// Push constants
	if (pushConstants.size() > 0) {
		pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
	}

	Instance::CheckVkResult("Create pipeline layout", device->CreatePipelineLayout(&pipelineLayoutInfo, nullptr, obj));
	
	// Descriptor sets array
	for (size_t i = 0; i < rawDescriptorSets.size(); ++i) {
		auto& rawSets = rawDescriptorSets[i];
		rawSets.reserve(descriptorSets.size());
		for (auto[set,framebufferedSet] : descriptorSets) {
			if (set) {
				rawSets.push_back(set->obj);
			} else if (framebufferedSet) {
				rawSets.push_back((*framebufferedSet)[i].obj);
			}
		}
	}
}

void PipelineLayoutObject::Destroy() {
	if (device) {
		for (auto& sets : rawDescriptorSets) sets.clear();
		device->DestroyPipelineLayout(obj, nullptr);
		device = nullptr;
	}
}
