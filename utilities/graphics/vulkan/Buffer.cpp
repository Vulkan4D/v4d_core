#include "Buffer.h"

namespace v4d::graphics::vulkan {

	void Buffer::Allocate(Device* device) {
		std::lock_guard lock(allocationMutex);
		if (this->device == nullptr) {
			assert(device);
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
				
				if (pool) {
					if (device->CreateAndAllocateBuffer(bufferInfo, pool, handle, &allocation) != VK_SUCCESS) {
						throw std::runtime_error("Failed to Create/Allocate Buffer");
					}
				} else {
					if (device->CreateAndAllocateBuffer(bufferInfo, memoryUsage, handle, &allocation) != VK_SUCCESS) {
						throw std::runtime_error("Failed to Create/Allocate Buffer");
					}
				}
				
				if (device->GetPhysicalDevice()->deviceFeatures.vulkan12DeviceFeatures.bufferDeviceAddress) {
					address = device->GetBufferDeviceOrHostAddressConst(handle);
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

	void Buffer::Free() {
		std::lock_guard lock(allocationMutex);
		if (device) {
			if (handle) {
				device->FreeAndDestroyBuffer(handle, allocation);
			}
			device = nullptr;
		}
	}

	Buffer::~Buffer() {
		Free();
	}
	
}
