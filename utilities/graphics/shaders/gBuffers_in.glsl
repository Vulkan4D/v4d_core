#include "GBuffers.glsl"

layout(set = 1, input_attachment_index = 1, binding = 0) uniform highp subpassInput gBuffer_albedo;
layout(set = 1, input_attachment_index = 2, binding = 1) uniform highp  subpassInput gBuffer_normal;
layout(set = 1, input_attachment_index = 3, binding = 2) uniform highp subpassInput gBuffer_emission;
layout(set = 1, input_attachment_index = 4, binding = 3) uniform highp subpassInput gBuffer_position;

GBuffers LoadGBuffers() {
	vec4 norm = subpassLoad(gBuffer_normal);
	vec4 emission = subpassLoad(gBuffer_emission);
	vec4 pos = subpassLoad(gBuffer_position);
	uint roughnessAndMetallic = floatBitsToUint(norm.w);
	uint scatterAndOcclusion = floatBitsToUint(emission.w);
	return GBuffers(
		subpassLoad(gBuffer_albedo),
		norm.xyz,
		emission.rgb,
		pos.xyz,
		pos.w,
		(roughnessAndMetallic & 0xff00) >> 8,
		(roughnessAndMetallic & 0x00ff),
		(scatterAndOcclusion & 0xff00) >> 8,
		(scatterAndOcclusion & 0x00ff)
	);
}
