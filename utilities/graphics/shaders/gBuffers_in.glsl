#include "GBuffers.glsl"

layout(set = 1, input_attachment_index = 1, binding = 0) uniform highp subpassInput gBuffer_albedo;
layout(set = 1, input_attachment_index = 2, binding = 1) uniform lowp  subpassInput gBuffer_normal;
layout(set = 1, input_attachment_index = 3, binding = 2) uniform lowp  subpassInput gBuffer_roughness;
layout(set = 1, input_attachment_index = 4, binding = 3) uniform lowp  subpassInput gBuffer_metallic;
layout(set = 1, input_attachment_index = 5, binding = 4) uniform lowp  subpassInput gBuffer_scatter;
layout(set = 1, input_attachment_index = 6, binding = 5) uniform lowp  subpassInput gBuffer_occlusion;
layout(set = 1, input_attachment_index = 7, binding = 6) uniform highp subpassInput gBuffer_emission;
layout(set = 1, input_attachment_index = 8, binding = 7) uniform highp subpassInput gBuffer_position;

GBuffers LoadGBuffers() {
	return GBuffers(
		subpassLoad(gBuffer_albedo).rgba,
		subpassLoad(gBuffer_normal).xyz,
		subpassLoad(gBuffer_roughness).s,
		subpassLoad(gBuffer_metallic).s,
		subpassLoad(gBuffer_scatter).s,
		subpassLoad(gBuffer_occlusion).s,
		subpassLoad(gBuffer_emission).rgb,
		subpassLoad(gBuffer_position).xyzw
	);
}
