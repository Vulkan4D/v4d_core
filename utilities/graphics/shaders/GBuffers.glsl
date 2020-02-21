struct GBuffers {
	highp vec4 albedo;
	highp vec3 normal;
	highp vec3 emission;
	highp vec3 position;
	highp float dist;
	lowp float roughness;
	lowp float metallic;
	lowp float scatter;
	lowp float occlusion;
};
