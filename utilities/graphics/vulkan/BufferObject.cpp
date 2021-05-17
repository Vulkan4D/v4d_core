#include "BufferObject.h"

namespace v4d::graphics::vulkan {

	COMMON_OBJECT_CPP(BufferObject, VkBuffer)

	void BufferObject::Allocate(Device* device) {
		if (this->device == nullptr) {
			this->device = device;
			if (size > 0) {
				VkBufferCreateInfo bufferInfo {};{
					bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
					bufferInfo.size = size;
					bufferInfo.usage = bufferUsage | (device->GetPhysicalDevice()->deviceFeatures.vulkan12DeviceFeatures.bufferDeviceAddress? VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT : 0);
					bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
					bufferInfo.queueFamilyIndexCount = 0;
					bufferInfo.pQueueFamilyIndices = nullptr;
				}
				device->CreateAndAllocateBuffer(bufferInfo, memoryUsage, obj, &allocation);
				if (device->GetPhysicalDevice()->deviceFeatures.vulkan12DeviceFeatures.bufferDeviceAddress) {
					address = device->GetBufferDeviceOrHostAddressConst(obj);
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
