#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

#define RAY_TRACING
// #define RENDER_LIGHT_SPHERES_MANUALLY_IN_RGEN

// Ray Tracing Payload
struct RayPayload_visibility {
	vec3 viewSpacePosition;
	vec3 viewSpaceNormal;
	vec3 albedo;
	float emit;
	vec2 uv;
	float metallic;
	float roughness;
	float distance;
};

// struct RayPayload_lighting {
// 	vec3 color;
// 	vec3 origin;
// 	vec3 direction;
// 	float distance;
// };

#include "v4d/core/utilities/graphics/generic/geometry_attributes.hh"
