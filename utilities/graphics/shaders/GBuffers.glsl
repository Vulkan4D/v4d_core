struct GBuffers {
	highp vec4 albedo;
	lowp  vec3 normal;
	lowp float roughness;
	lowp float metallic;
	lowp float scatter;
	lowp float occlusion;
	highp vec3 emission;
	highp vec4 position;
};
