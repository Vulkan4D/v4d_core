#pragma once

#include <v4d.h>
#include <vector>
#include <map>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Shader.h"
#include "utilities/graphics/vulkan/PipelineLayoutObject.h"
#include "utilities/graphics/vulkan/BufferObject.h"
#include "utilities/graphics/Renderer.h"
#include "RayTracingPipeline.h"

namespace v4d::graphics::vulkan::raytracing {
	using namespace v4d::graphics::vulkan;
	
	// DEPRECATED
	class V4DLIB ShaderBindingTable : public RayTracingPipeline {
	public:
		using RayTracingPipeline::RayTracingPipeline;
		void WriteShaderBindingTableToBuffer();
		void Create(Device* device);
	};
}
