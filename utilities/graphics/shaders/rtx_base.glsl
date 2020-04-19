#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

// Settings
// const int reflection_max_recursion = 3;

// #define RENDER_LIGHT_SPHERES_MANUALLY_IN_RGEN

// Descriptor Set 0
#include "Camera.glsl"

// Descriptor Set 1
layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = 1, rgba16f) uniform image2D litImage;
layout(set = 1, binding = 2, r32f) uniform image2D depthImage;
layout(set = 1, binding = 3, rg32f) uniform image2D gBuffer_albedo_geometryIndex;
layout(set = 1, binding = 4, rgba32f) uniform image2D gBuffer_normal_uv;
layout(set = 1, binding = 5, rgba32f) uniform image2D gBuffer_position_dist;

// Ray Tracing Payload
struct RayPayload {
	vec3 color;
	vec3 origin;
	vec3 direction;
	float distance;
	float opacity;
	// float reflector;
};

#define SOLID 0x01
#define LIQUID 0x02
#define CLOUD 0x04
#define PARTICLE 0x08
#define TRANSPARENT 0x10
#define CUTOUT 0x20
#define CELESTIAL 0x40
#define EMITTER 0x80

#define RAY_TRACING
#include "core_buffers.glsl"
