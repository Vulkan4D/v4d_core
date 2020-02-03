#include "GBuffers.glsl"

layout(location = 0) out highp vec4 gBuffer_albedo;
layout(location = 1) out lowp  vec3 gBuffer_normal;
layout(location = 2) out lowp float gBuffer_roughness;
layout(location = 3) out lowp float gBuffer_metallic;
layout(location = 4) out lowp float gBuffer_scatter;
layout(location = 5) out lowp float gBuffer_occlusion;
layout(location = 6) out highp vec3 gBuffer_emission;
layout(location = 7) out highp vec4 gBuffer_position;

void WriteGBuffers(GBuffers gBuffers) {
	gBuffer_albedo = gBuffers.albedo;
	gBuffer_normal = gBuffers.normal;
	gBuffer_roughness = gBuffers.roughness;
	gBuffer_metallic = gBuffers.metallic;
	gBuffer_scatter = gBuffers.scatter;
	gBuffer_occlusion = gBuffers.occlusion;
	gBuffer_emission = gBuffers.emission;
	gBuffer_position = gBuffers.position;
}
