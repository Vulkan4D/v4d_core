#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

#define RAY_TRACING
// #define RENDER_LIGHT_SPHERES_MANUALLY_IN_RGEN

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
