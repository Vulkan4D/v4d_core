#include "GBuffers.glsl"

layout(location = 0) out highp vec4 gBuffer_albedo;
layout(location = 1) out highp vec4 gBuffer_normal;
layout(location = 2) out highp vec4 gBuffer_emission;
layout(location = 3) out highp vec4 gBuffer_position;

void WriteGBuffers(GBuffers gBuffers) {
	gBuffer_albedo = gBuffers.albedo;
	gBuffer_normal = vec4(gBuffers.normal, uintBitsToFloat((uint(gBuffers.roughness*255) << 8) + uint(gBuffers.metallic*255)));
	gBuffer_emission = vec4(gBuffers.emission, uintBitsToFloat((uint(gBuffers.scatter*255) << 8) + uint(gBuffers.occlusion*255)));
	gBuffer_position = vec4(gBuffers.position, gBuffers.dist);
}
