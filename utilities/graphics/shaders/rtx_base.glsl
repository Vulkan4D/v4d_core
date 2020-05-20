#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

#if defined(SHADER_RGEN) || defined(SHADER_RMISS) || defined(SHADER_RCHIT) || defined(SHADER_RAHIT) || defined(SHADER_RINT)
	#define RAY_TRACING
		
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

#else
	#define RAY_QUERY
	#extension GL_EXT_ray_query : enable
#endif

#include "v4d/core/utilities/graphics/generic/geometry_attributes.hh"
