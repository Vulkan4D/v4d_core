#include <v4d.h>

using namespace v4d::graphics::vulkan;

DescriptorBinding::~DescriptorBinding() {
	if (writeInfo) {
		switch (pointerType) {
			case STORAGE_BUFFER:
				delete[] (VkDescriptorBufferInfo*)writeInfo;
			break;
			case UNIFORM_BUFFER:
				delete[] (VkDescriptorBufferInfo*)writeInfo;
			break;
			case IMAGE_VIEW:
			case COMBINED_IMAGE_SAMPLER:
			case INPUT_ATTACHMENT:
			case INPUT_ATTACHMENT_DEPTH_STENCIL:
				delete[] (VkDescriptorImageInfo*)writeInfo;
			break;
			case ACCELERATION_STRUCTURE:
				delete (VkWriteDescriptorSetAccelerationStructureNV*)writeInfo;
			break;
			// default: throw std::runtime_error("pointerType is not implemented in destructor");
		}
	}
}

bool DescriptorBinding::IsWriteDescriptorSetValid() const {
	switch (pointerType) {
		case STORAGE_BUFFER:
		case UNIFORM_BUFFER:
			return ((Buffer*)data)->buffer != VK_NULL_HANDLE;
		break;
		case IMAGE_VIEW:
		case INPUT_ATTACHMENT:
		case INPUT_ATTACHMENT_DEPTH_STENCIL:
			return ((Image*)data)->view != VK_NULL_HANDLE;
		break;
		case COMBINED_IMAGE_SAMPLER:
			return ((Image*)data)->sampler != VK_NULL_HANDLE && ((Image*)data)->view != VK_NULL_HANDLE;
		break;
		case ACCELERATION_STRUCTURE:
			return *(VkAccelerationStructureNV*)data != VK_NULL_HANDLE;
		break;
	}
	return false;
}

VkWriteDescriptorSet DescriptorBinding::GetWriteDescriptorSet(VkDescriptorSet descriptorSet) {
	VkWriteDescriptorSet descriptorWrite {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstSet = descriptorSet;
	
	descriptorWrite.dstArrayElement = dstArrayElement;
	descriptorWrite.descriptorCount = descriptorCount;
	descriptorWrite.descriptorType = descriptorType;
	
	switch (pointerType) {
		case STORAGE_BUFFER:
			writeInfo = new VkDescriptorBufferInfo[descriptorCount];
			((VkDescriptorBufferInfo*)writeInfo)[0] = {
				((Buffer*)data)[0].buffer,// VkBuffer buffer
				0,// VkDeviceSize offset
				((Buffer*)data)[0].size,// VkDeviceSize range
			};
			descriptorWrite.pBufferInfo = (VkDescriptorBufferInfo*)writeInfo;
		break;
		case UNIFORM_BUFFER:
			writeInfo = new VkDescriptorBufferInfo[descriptorCount];
			((VkDescriptorBufferInfo*)writeInfo)[0] = {
				((Buffer*)data)[0].buffer,// VkBuffer buffer
				0,// VkDeviceSize offset
				((Buffer*)data)[0].size,// VkDeviceSize range
			};
			descriptorWrite.pBufferInfo = (VkDescriptorBufferInfo*)writeInfo;
		break;
		case IMAGE_VIEW:
			writeInfo = new VkDescriptorImageInfo[descriptorCount];
			((VkDescriptorImageInfo*)writeInfo)[0] = {
				VK_NULL_HANDLE,// VkSampler sampler
				((Image*)data)[0].view,// VkImageView imageView
				VK_IMAGE_LAYOUT_GENERAL,// VkImageLayout imageLayout
			};
			descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)writeInfo;
		break;
		case COMBINED_IMAGE_SAMPLER:
			writeInfo = new VkDescriptorImageInfo[descriptorCount];
			((VkDescriptorImageInfo*)writeInfo)[0] = {
				((Image*)data)[0].sampler,// VkSampler sampler
				((Image*)data)[0].view,// VkImageView imageView
				VK_IMAGE_LAYOUT_GENERAL,// VkImageLayout imageLayout
			};
			descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)writeInfo;
		break;
		case INPUT_ATTACHMENT:
			writeInfo = new VkDescriptorImageInfo[descriptorCount];
			((VkDescriptorImageInfo*)writeInfo)[0] = {
				VK_NULL_HANDLE,// VkSampler sampler
				((Image*)data)[0].view,// VkImageView imageView
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,// VkImageLayout imageLayout
			};
			descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)writeInfo;
		break;
		case INPUT_ATTACHMENT_DEPTH_STENCIL:
			writeInfo = new VkDescriptorImageInfo[descriptorCount];
			((VkDescriptorImageInfo*)writeInfo)[0] = {
				VK_NULL_HANDLE,// VkSampler sampler
				((Image*)data)[0].view,// VkImageView imageView
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,// VkImageLayout imageLayout
			};
			descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)writeInfo;
		break;
		case ACCELERATION_STRUCTURE:
			writeInfo = new VkWriteDescriptorSetAccelerationStructureNV {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV,// VkStructureType sType
				nullptr,// const void* pNext
				descriptorCount,// uint32_t accelerationStructureCount
				(VkAccelerationStructureNV*)data,// const VkAccelerationStructureNV* pAccelerationStructures
			};
			descriptorWrite.pNext = (VkWriteDescriptorSetAccelerationStructureNV*)writeInfo;
		break;
		// default: throw std::runtime_error("pointerType is not implemented in GetWriteDescriptorSet()");
	}
	return descriptorWrite;
}

DescriptorSet::DescriptorSet(uint32_t set) : set(set) {}

std::map<uint32_t, DescriptorBinding>& DescriptorSet::GetBindings() {
	return bindings;
}

VkDescriptorSetLayout DescriptorSet::GetDescriptorSetLayout() const {
	return descriptorSetLayout;
}

void DescriptorSet::CreateDescriptorSetLayout(Device* device) {
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings {};
	for (auto& [binding, set] : bindings) {
		layoutBindings.push_back({binding, set.descriptorType, set.descriptorCount, set.stageFlags, set.pImmutableSamplers});
	}
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();
	
	if (device->CreateDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
}

void DescriptorSet::DestroyDescriptorSetLayout(Device* device) {
	device->DestroyDescriptorSetLayout(descriptorSetLayout, nullptr);
	descriptorSetLayout = VK_NULL_HANDLE;
}
