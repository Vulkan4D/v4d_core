#include "BufferObject.h"

namespace v4d::graphics::vulkan {

	COMMON_OBJECT_CPP(BufferObject, VkBuffer)

	void BufferObject::Allocate(Device* device) {
		if (this->device == nullptr) {
			this->device = device;
			if (size > 0) {
				VkBufferCreateInfo bufferInfo {};{
					bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
					bufferInfo.size = size + alignment;
					bufferInfo.usage = bufferUsage;
					bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
					bufferInfo.queueFamilyIndexCount = 0;
					bufferInfo.pQueueFamilyIndices = nullptr;
				}
				
				if (device->GetPhysicalDevice()->deviceFeatures.vulkan12DeviceFeatures.bufferDeviceAddress)
					bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				
				device->CreateAndAllocateBuffer(bufferInfo, memoryUsage, obj, &allocation);
				if (device->GetPhysicalDevice()->deviceFeatures.vulkan12DeviceFeatures.bufferDeviceAddress) {
					address = device->GetBufferDeviceOrHostAddressConst(obj);
					if (alignment > 0 && address.deviceAddress % alignment != 0) {
						alignedOffset = ((address.deviceAddress + (alignment - 1)) & ~(alignment - 1)) - address.deviceAddress;
						address.deviceAddress += alignedOffset;
						assert(alignedOffset > 0);
						assert(alignedOffset < alignment);
						assert(address.deviceAddress % alignment == 0);
					} else {
						alignedOffset = 0;
					}
				}
			}
		}
	}

	void BufferObject::Free() {
		if (device) {
			if ((VkBuffer)obj != VK_NULL_HANDLE) {
				device->FreeAndDestroyBuffer(obj, allocation);
			}
			device = nullptr;
		}
	}

	BufferObject::~BufferObject() {
		Free();
	}
	
}
