#pragma once
#include <v4d.h>

class V4DLIB V4D_Mod {
	V4D_MODULE_CLASS_HEADER(V4D_Mod
		,OrderIndex
		,LoadScene
		,UnloadScene
		,DrawUi
		,InitRenderer
		,InitClient
		,InitServer
		,InitWindow
		,InputCallbackName
		,InputCharCallback
		,InputKeyCallback
		,InputScrollCallback
		,MouseButtonCallback
		,InputUpdate
		,GameLoopUpdate
		,SlowLoopUpdate
		,CreateVulkanResources2
		,DestroyVulkanResources2
		,DrawUi2
		,DrawUiDebug2
		,BeginSecondaryFrameUpdate
		,SecondaryFrameCompute
		,OnRendererRayCastHit
		,OnRendererRayCastOut
		,PhysicsUpdate
		,RenderOrderIndex
		,ScorePhysicalDeviceSelection
		,InitVulkanDeviceFeatures
		,ConfigureRenderer
		,InitVulkanLayouts
		,InitRenderingDevice
		,ConfigureShaders
		,ReadShaders
		,CreateVulkanSyncObjects
		,DestroyVulkanSyncObjects
		,CreateVulkanResources
		,DestroyVulkanResources
		,CreateVulkanCommandBuffers
		,DestroyVulkanCommandBuffers
		,AllocateVulkanBuffers
		,FreeVulkanBuffers
		,CreateVulkanPipelines
		,DestroyVulkanPipelines
		,RenderUpdate
		,SecondaryRenderUpdate
		
		,RenderFrame_BeforeUpdate
		,RenderFrame_Update
		,RenderFrame_Push
		,RenderFrame_Build
		,RenderFrame_Graphics
		,RenderFrame_Physics
		,RenderFrame_Post
		,RenderFrame_Pull
		
		,SecondaryRenderUpdate2
		,DrawOverlayLine
		,DrawOverlayText
		,DrawOverlayCircle
		,DrawOverlaySquare
		,GetImage
		,GetPipelineLayout
		,AddShader
		,AddRayTracingHitShader
		,GetShaderBindingTable
		,CreateGameObject
		,DestroyGameObject
		,SendStreamCustomGameObjectData
		,ReceiveStreamCustomGameObjectData
		,SendStreamCustomGameObjectTransformData
		,ReceiveStreamCustomGameObjectTransformData
		,AddGameObjectToScene
		,ClientSendActions
		,ClientSendBursts
		,ClientReceiveAction
		,ClientReceiveBurst
		,ServerIncomingClient
		,ServerSendActions
		,ServerSendBursts
		,ServerReceiveAction
		,ServerReceiveBurst
	)
	
// Init
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, LoadScene, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, UnloadScene)
	V4D_MODULE_FUNC_DECLARE(void, InitRenderer, v4d::graphics::Renderer*)
	V4D_MODULE_FUNC_DECLARE(void, InitClient, std::shared_ptr<v4d::networking::OutgoingConnection> client)
	V4D_MODULE_FUNC_DECLARE(void, InitServer, std::shared_ptr<v4d::networking::ListeningServer> server)
	V4D_MODULE_FUNC_DECLARE(void, InitWindow, v4d::graphics::Window*)

// Input
	V4D_MODULE_FUNC_DECLARE(std::string, InputCallbackName)
	V4D_MODULE_FUNC_DECLARE(void, InputCharCallback, unsigned int c)
	V4D_MODULE_FUNC_DECLARE(void, InputKeyCallback, int key, int scancode, int action, int mods)
	V4D_MODULE_FUNC_DECLARE(void, InputScrollCallback, double x, double y)
	V4D_MODULE_FUNC_DECLARE(void, MouseButtonCallback, int button, int action, int mods)
	V4D_MODULE_FUNC_DECLARE(void, InputUpdate, double deltaTime)
	
// Game
	V4D_MODULE_FUNC_DECLARE(void, GameLoopUpdate, double deltaTime)
	V4D_MODULE_FUNC_DECLARE(void, SlowLoopUpdate, double deltaTime)
	V4D_MODULE_FUNC_DECLARE(void, CreateVulkanResources2, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC_DECLARE(void, DestroyVulkanResources2, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC_DECLARE(void, DrawUi2)
	V4D_MODULE_FUNC_DECLARE(void, DrawUiDebug2)
	V4D_MODULE_FUNC_DECLARE(void, BeginSecondaryFrameUpdate)
	V4D_MODULE_FUNC_DECLARE(void, SecondaryFrameCompute, VkCommandBuffer)
	V4D_MODULE_FUNC_DECLARE(void, OnRendererRayCastHit, v4d::graphics::RayCast)
	V4D_MODULE_FUNC_DECLARE(void, OnRendererRayCastOut, v4d::graphics::RayCast)

// Physics
	V4D_MODULE_FUNC_DECLARE(void, PhysicsUpdate, double deltaTime)

// Renderer
	// Executed only once for configuration
	V4D_MODULE_FUNC_DECLARE(int, RenderOrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, ScorePhysicalDeviceSelection, int& score, v4d::graphics::vulkan::PhysicalDevice*) // this is executed once for each available GPU
	// May be Executed many times (ie: when renderer is reloading or screen is resizing)
	V4D_MODULE_FUNC_DECLARE(void, InitVulkanDeviceFeatures, v4d::graphics::vulkan::PhysicalDevice::DeviceFeatures* deviceFeaturesToEnable, const v4d::graphics::vulkan::PhysicalDevice::DeviceFeatures* supportedDeviceFeatures)
	V4D_MODULE_FUNC_DECLARE(void, ConfigureRenderer)
	V4D_MODULE_FUNC_DECLARE(void, InitVulkanLayouts)
	V4D_MODULE_FUNC_DECLARE(void, InitRenderingDevice, v4d::graphics::vulkan::Device*)
	V4D_MODULE_FUNC_DECLARE(void, ConfigureShaders)
	V4D_MODULE_FUNC_DECLARE(void, ReadShaders)
	V4D_MODULE_FUNC_DECLARE(void, CreateVulkanSyncObjects)
	V4D_MODULE_FUNC_DECLARE(void, DestroyVulkanSyncObjects)
	V4D_MODULE_FUNC_DECLARE(void, CreateVulkanResources)
	V4D_MODULE_FUNC_DECLARE(void, DestroyVulkanResources)
	V4D_MODULE_FUNC_DECLARE(void, CreateVulkanCommandBuffers)
	V4D_MODULE_FUNC_DECLARE(void, DestroyVulkanCommandBuffers)
	V4D_MODULE_FUNC_DECLARE(void, AllocateVulkanBuffers)
	V4D_MODULE_FUNC_DECLARE(void, FreeVulkanBuffers)
	V4D_MODULE_FUNC_DECLARE(void, CreateVulkanPipelines)
	V4D_MODULE_FUNC_DECLARE(void, DestroyVulkanPipelines)
	
	// Executed every frame
	V4D_MODULE_FUNC_DECLARE(void, DrawUi) // Main Renderer module must call DrawUi2 and DrawUiDebug2
	V4D_MODULE_FUNC_DECLARE(void, RenderUpdate) // Main Renderer module must call all the RenderFrame_* functions for all modules
	V4D_MODULE_FUNC_DECLARE(void, SecondaryRenderUpdate) // Main Renderer module must call BeginSecondaryFrameUpdate and SecondaryFrameCompute and SecondaryRenderUpdate2
	
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_BeforeUpdate)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Update)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Push, VkCommandBuffer/*Transfer queue*/)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Build, VkCommandBuffer/*Compute queue*/)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Graphics, VkCommandBuffer/*Graphics queue*/)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Physics, VkCommandBuffer/*Compute queue*/)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Post, VkCommandBuffer/*Graphics queue*/)
	V4D_MODULE_FUNC_DECLARE(void, RenderFrame_Pull, VkCommandBuffer/*Transfer queue*/)
	
	V4D_MODULE_FUNC_DECLARE(void, SecondaryRenderUpdate2, VkCommandBuffer)
	// Typically only implemented in primary rendering module
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayLine, float x1, float y1, float x2, float y2, glm::vec4 color, float lineWidth)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayText, const char* text, float x, float y, glm::vec4 color, float size)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlayCircle, float x, float y, glm::vec4 color, float size, float borderSize)
	V4D_MODULE_FUNC_DECLARE(void, DrawOverlaySquare, float x, float y, glm::vec4 color, glm::vec2 size, float borderSize)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::Image*, GetImage, const std::string& name)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::PipelineLayout*, GetPipelineLayout, const std::string& name)
	V4D_MODULE_FUNC_DECLARE(void, AddShader, const std::string& groupName, v4d::graphics::vulkan::RasterShaderPipeline* shader)
	V4D_MODULE_FUNC_DECLARE(void, AddRayTracingHitShader, v4d::modular::ModuleID, const std::string& name)
	V4D_MODULE_FUNC_DECLARE(v4d::graphics::vulkan::rtx::ShaderBindingTable*, GetShaderBindingTable, const std::string& sbtName)

// GameObjects
	V4D_MODULE_FUNC_DECLARE(void, CreateGameObject, v4d::scene::NetworkGameObjectPtr obj)
	V4D_MODULE_FUNC_DECLARE(void, DestroyGameObject, v4d::scene::NetworkGameObjectPtr obj, v4d::scene::Scene* scene)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomGameObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::WriteOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomGameObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::ReadOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomGameObjectTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::WriteOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomGameObjectTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::ReadOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, AddGameObjectToScene, v4d::scene::NetworkGameObjectPtr obj, v4d::scene::Scene* scene)

// Client
	V4D_MODULE_FUNC_DECLARE(void, ClientSendActions, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ClientSendBursts, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ClientReceiveAction, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ClientReceiveBurst, v4d::io::SocketPtr stream)

// Server
	V4D_MODULE_FUNC_DECLARE(void, ServerIncomingClient, v4d::networking::IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ServerSendActions, v4d::io::SocketPtr stream, v4d::networking::IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ServerSendBursts, v4d::io::SocketPtr stream, v4d::networking::IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ServerReceiveAction, v4d::io::SocketPtr stream, v4d::networking::IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ServerReceiveBurst, v4d::io::SocketPtr stream, v4d::networking::IncomingClientPtr client)


};
