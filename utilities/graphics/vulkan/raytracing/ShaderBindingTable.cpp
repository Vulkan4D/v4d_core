#include "ShaderBindingTable.h"

using namespace v4d::graphics::vulkan;
using namespace v4d::graphics::vulkan::raytracing;

void ShaderBindingTable::WriteShaderBindingTableToBuffer() {
	uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	uint64_t alignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t stride = handleSize;
	if (alignment > 0) stride = (handleSize + (alignment - 1)) & ~(alignment - 1);
	assert(stride >= handleSize);
	assert(stride >= alignment);
	
	VkDeviceAddress baseAddress = *pipelineBuffer;
	// Align
	size_t bufferOffset = 0;
	if (alignment > 0) {
		bufferOffset = ((baseAddress + (alignment - 1)) & ~(alignment - 1)) - baseAddress;
	}
	
	// Ray Gen
	rayGenDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayGenShaderRegionOffset,
		/*stride*/rayGenShaderRegionSize,
		/*size*/rayGenShaderRegionSize
	};
	
	// Ray Miss
	rayMissDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayMissShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayMissShaderRegionSize
	};
	
	// Ray Callable
	rayCallableDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayCallableShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayCallableShaderRegionSize
	};
	
	// Ray Hit
	rayHitDeviceAddressRegion = {
		/*deviceAddress*/baseAddress + bufferOffset + rayHitShaderRegionOffset,
		/*stride*/stride,
		/*size*/rayHitShaderRegionSize
	};
	
	assert(allGroups.size() > 0);
	const size_t shaderHandlerStorageSize = allGroups.size()*stride;
	auto shaderHandleStorage = new uint8_t[shaderHandlerStorageSize];
	if (device->GetRayTracingShaderGroupHandlesKHR(pipeline, 0, (uint)allGroups.size(), shaderHandlerStorageSize, shaderHandleStorage) != VK_SUCCESS)
		throw std::runtime_error("Failed to get ray tracing shader group handles");
	
	pipelineBuffer->ZeroInitialize();
	
	// Ray Gen
	pipelineBuffer->Fill(
		/*SrcPtr*/shaderHandleStorage,
		/*SizeBytes*/handleSize,
		/*OffsetBytes*/bufferOffset + rayGenShaderRegionOffset
	);
	
	uint32_t shaderHandlerStorageOffset = handleSize * rayGenGroups.size();
	
	// Ray Miss
	for (size_t i = 0; i < rayMissGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayMissShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Callable
	for (size_t i = 0; i < rayCallableGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayCallableShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	// Ray Hit
	for (size_t i = 0; i < rayHitGroups.size(); ++i) {
		pipelineBuffer->Fill(
			/*SrcPtr*/shaderHandleStorage + shaderHandlerStorageOffset,
			/*SizeBytes*/handleSize,
			/*OffsetBytes*/bufferOffset + rayHitShaderRegionOffset + stride * i
		);
		shaderHandlerStorageOffset += handleSize;
	}
	
	delete[] shaderHandleStorage;
	dirty = true;
}

void ShaderBindingTable::Create(Device* device) {
	if (this->device == nullptr) {
		this->device = device;
		CreateRayTracingPipeline();
		WriteShaderBindingTableToBuffer();
	}
}
