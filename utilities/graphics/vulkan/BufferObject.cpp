#include "BufferObject.h"

namespace v4d::graphics::vulkan {

	COMMON_OBJECT_CPP(BufferObject, VkBuffer)

	void BufferObject::Allocate(Device* device) {
		this->device = device;
		VkBufferCreateInfo bufferInfo {};{
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = bufferUsage;
			bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			bufferInfo.queueFamilyIndexCount = 0;
			bufferInfo.pQueueFamilyIndices = nullptr;
		}
		device->CreateAndAllocateBuffer(bufferInfo, memoryUsage, obj, &allocation);
	}

	void BufferObject::Free() {
		assert(device);
		if ((VkBuffer)obj != VK_NULL_HANDLE) {
			device->FreeAndDestroyBuffer(obj, allocation);
		}
		device = nullptr;
	}

}
