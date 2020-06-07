#pragma once

// Powerful utilities deeply integrated with V4D (Compiled into v4d.dll)

#include "utilities/io/Logger.h"
#include "utilities/processing/ThreadPool.h"
#include "utilities/crypto/Crypto.h"
#include "utilities/crypto/AES.h"
#include "utilities/crypto/RSA.h"
#include "utilities/crypto/SHA.h"
#include "utilities/crypto/Random.h"
#include "utilities/data/Stream.h"
#include "utilities/data/DataStream.hpp"
#include "utilities/data/ReadOnlyStream.hpp"
#include "utilities/data/WriteOnlyStream.hpp"
#include "utilities/io/FilePath.h"
#include "utilities/io/BinaryFileStream.h"
#include "utilities/io/ConfigFile.h"
#include "utilities/io/ASCIIFile.h"
#include "utilities/io/TextFile.h"
#include "utilities/io/StringListFile.h"
#include "utilities/io/Socket.h"
#include "utilities/networking/ZAP.hh"
#include "utilities/networking/OutgoingConnection.h"
#include "utilities/networking/IncomingClient.h"
#include "utilities/networking/ListeningServer.h"

// Vulkan Graphics utilities
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/vulkan/PhysicalDevice.h"
#include "utilities/graphics/vulkan/Queue.hpp"
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/vulkan/Instance.h"
#include "utilities/graphics/vulkan/Image.h"
#include "utilities/graphics/vulkan/SwapChain.h"
#include "utilities/graphics/vulkan/Buffer.h"
#include "utilities/graphics/vulkan/DescriptorSet.h"
#include "utilities/graphics/vulkan/PipelineLayout.h"
#include "utilities/graphics/vulkan/Shader.h"
#include "utilities/graphics/vulkan/ShaderProgram.h"
#include "utilities/graphics/vulkan/RenderPass.h"
#include "utilities/graphics/vulkan/ShaderPipeline.h"
#include "utilities/graphics/vulkan/ComputeShaderPipeline.h"
#include "utilities/graphics/vulkan/RasterShaderPipeline.h"
// Ray Tracing
#include "utilities/graphics/vulkan/rtx/ShaderBindingTable.h"

// General Graphics utilities
#include "utilities/graphics/Window.h"
#include "utilities/graphics/Renderer.h"

// Ray Tracing
#include "utilities/graphics/vulkan/rtx/AccelerationStructure.h"

// Scene-related Objects
#include "utilities/scene/Scene.h"
#include "utilities/scene/NetworkGameObject.hpp"

// ImGui
#ifdef _ENABLE_IMGUI
	#ifndef IMGUI_API
		#define IMGUI_API V4DLIB
		#define IMGUI_IMPL_API V4DLIB
	#endif
	#include "imgui/imgui.h"
	#include "utilities/graphics/imgui_vulkan.h"
	#include "imgui/examples/imgui_impl_glfw.h"
#endif

// V4D Renderer
//...
