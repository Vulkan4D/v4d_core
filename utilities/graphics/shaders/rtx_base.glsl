#extension GL_EXT_ray_tracing : require

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

// Settings
// const int reflection_max_recursion = 3;

// #define RENDER_LIGHT_SPHERES_MANUALLY_IN_RGEN

// Descriptor Set 0
#include "Camera.glsl"
layout(set = 0, binding = 1) readonly buffer ObjectBuffer {mat4 objectInstances[];};
layout(set = 0, binding = 2) readonly buffer LightBuffer {float lightSources[];};
layout(set = 0, binding = 3) readonly buffer ActiveLights {
	uint activeLights;
	uint lightIndices[];
};

// Descriptor Set 1
layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = 1, rgba16f) uniform image2D litImage;
layout(set = 1, binding = 2, r32f) uniform image2D depthImage;
layout(set = 1, binding = 3) readonly buffer GeometryBuffer {uvec4 geometries[];};
layout(set = 1, binding = 4) readonly buffer IndexBuffer {uint indices[];};
layout(set = 1, binding = 5) readonly buffer VertexBuffer {vec4 vertices[];};

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

struct LightSource {
	vec3 position;
	float intensity;
	vec3 color;
	uint type;
	uint attributes;
	float radius;
	float custom1;
};

LightSource GetLight (uint i) {
	LightSource light;
	light.position = vec3(lightSources[i*8 + 0], lightSources[i*8 + 1], lightSources[i*8 + 2]);
	light.intensity = lightSources[i*8 + 3];
	vec4 c = UnpackColorFromFloat(lightSources[i*8 + 4]);
	light.color = c.rgb;
	light.type = floatBitsToUint(lightSources[i*8 + 4]) & 0x000000ff;
	light.attributes = floatBitsToUint(lightSources[i*8 + 5]);
	light.radius = lightSources[i*8 + 6];
	light.custom1 = lightSources[i*8 + 7];
	return light;
};

struct ObjectInstance {
	mat4 modelViewMatrix;
	mat3 normalMatrix;
	vec3 position;
	vec3 custom3;
	vec4 custom4;
};

ObjectInstance GetObjectInstance(uint index) {
	ObjectInstance obj;
	obj.modelViewMatrix = objectInstances[index*2];
	mat4 secondMatrix = objectInstances[index*2+1];
	obj.normalMatrix = mat3(
		secondMatrix[0].xyz,
		vec3(secondMatrix[0].w, secondMatrix[1].x, secondMatrix[1].y),
		vec3(secondMatrix[1].z, secondMatrix[1].w, secondMatrix[2].x)
	);
	obj.position = obj.modelViewMatrix[3].xyz;
	obj.custom3 = secondMatrix[2].yzw;
	obj.custom4 = objectInstances[index*2+1][3];
	return obj;
}

struct Vertex {
	vec3 pos;
	vec4 color;
	vec3 normal;
	vec2 uv;
};

Vertex GetVertex(uint index) {
	Vertex v;
	v.pos = vertices[index*2].xyz;
	v.color = UnpackColorFromFloat(vertices[index*2].w);
	v.normal = vertices[index*2+1].xyz;
	v.uv = UnpackUVfromFloat(vertices[index*2+1].w);
	return v;
}

struct ProceduralGeometry {
	uint vertexOffset;
	uint objectIndex;
	uint material;
	
	vec3 aabbMin;
	vec3 aabbMax;
	vec4 color;
	float custom1;
	
	ObjectInstance objectInstance;
};

ProceduralGeometry GetProceduralGeometry(uint geometryIndex) {
	ProceduralGeometry geom;
	
	geom.vertexOffset = geometries[geometryIndex].y;
	geom.objectIndex = geometries[geometryIndex].z;
	geom.material = geometries[geometryIndex].w;
	
	geom.aabbMin = vertices[geom.vertexOffset*2].xyz;
	geom.aabbMax = vec3(vertices[geom.vertexOffset*2].w, vertices[geom.vertexOffset*2+1].xy);
	geom.color = UnpackColorFromFloat(vertices[geom.vertexOffset*2+1].z);
	geom.custom1 = vertices[geom.vertexOffset*2+1].w;
	
	geom.objectInstance = GetObjectInstance(geom.objectIndex);
	
	return geom;
}
