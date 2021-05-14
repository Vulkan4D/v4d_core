#include "DescriptorSetObject.h"
#include "utilities/graphics/vulkan/Instance.h"

namespace v4d::graphics::vulkan {
	COMMON_OBJECT_CPP(DescriptorSetObject, VkDescriptorSet)
	DescriptorSetObject::Descriptor::~Descriptor() {}
	
	void DescriptorSetObject::CreateDescriptorSetLayout(Device* device) {
		assert(this->device == nullptr);
		this->device = device;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings {};
		for (auto& [binding, descriptor] : descriptorBindings) {
			layoutBindings.push_back({binding, descriptor->GetDescriptorType(), descriptor->count, descriptor->stageFlags, descriptor->GetImmutableSamplersPtr()});
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};{
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = layoutBindings.size();
			layoutInfo.pBindings = layoutBindings.data();
		}
		Instance::CheckVkResult("Create DescriptorSetLayout", device->CreateDescriptorSetLayout(&layoutInfo, nullptr, &layout));
	}
	
	void DescriptorSetObject::DestroyDescriptorSetLayout() {
		assert(device);
		device->DestroyDescriptorSetLayout(layout, nullptr);
		layout = VK_NULL_HANDLE;
		device = nullptr;
	}
	
	void DescriptorSetObject::Allocate(VkDescriptorPool descriptorPool) {
		assert(device);
		VkDescriptorSetAllocateInfo allocInfo = {};{
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;
		}
		Instance::CheckVkResult("Allocate DescriptorSet", device->AllocateDescriptorSets(&allocInfo, obj));
	}
	
	void DescriptorSetObject::Free(VkDescriptorPool descriptorPool) {
		assert(device);
		device->FreeDescriptorSets(descriptorPool, 1, obj);
	}
	
	std::vector<VkWriteDescriptorSet> DescriptorSetObject::GetUpdateWrites() {
		std::vector<VkWriteDescriptorSet> writes {};
		writes.reserve(descriptorBindings.size());
		VkWriteDescriptorSet write {};{
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = obj;
		}
		for (auto&[binding, descriptor] : descriptorBindings) {
			write.dstBinding = binding;
			write.descriptorType = descriptor->GetDescriptorType();
			write.descriptorCount = descriptor->count;
			descriptor->FillWriteInfo(write);
			writes.push_back(write);
		}
		isDirty = false;
		return writes;
	}
	
}
