layout(set = 1, input_attachment_index = 1, binding = 0) uniform highp subpassInput gBuffer_albedo_geometryIndex;
layout(set = 1, input_attachment_index = 2, binding = 1) uniform highp subpassInput gBuffer_normal_uv;
layout(set = 1, input_attachment_index = 3, binding = 2) uniform highp subpassInput gBuffer_position_dist;

layout(set = 1, binding = 3) uniform sampler2D depthImage;
layout(set = 1, binding = 4) uniform sampler2D rasterDepthImage;

layout(location = 0) out vec4 out_color;
