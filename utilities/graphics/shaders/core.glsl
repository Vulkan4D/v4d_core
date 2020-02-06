#version 460 core
#extension GL_ARB_enhanced_layouts : enable

precision highp int;
precision highp float;
precision highp sampler2D;

ivec4 UnpackIVec4FromUint(uint i) {
	return ivec4(
		(i & 0xff000000) >> 24,
		(i & 0x00ff0000) >> 16,
		(i & 0x0000ff00) >> 8,
		(i & 0x000000ff)
	);
}

vec4 UnpackVec4FromUint(uint i) {
	return vec4(
		((i & 0xff000000) >> 24) / 255.0,
		((i & 0x00ff0000) >> 16) / 255.0,
		((i & 0x0000ff00) >> 8)  / 255.0,
		((i & 0x000000ff)	  )  / 255.0
	);
}

vec3 UnpackVec3FromFloat(float f) {
	vec3 color;
	color.b = floor(f / 256.0 / 256.0);
	color.g = floor((f - color.b * 256.0 * 256.0) / 256.0);
	color.r = floor(f - color.b * 256.0 * 256.0 - color.g * 256.0);
	return color;
}

vec4 SampleTexture(sampler2D tex, vec2 fragCoord) {
	return texture(tex, fragCoord.st / vec2(textureSize(tex, 0)));
}


// float linearstep(float a, float b, float x) {
// 	if (b == a) return (x >= a ? 1 : 0);
// 	return (x - a) / (b - a);
// }
// // Ease Functions	https://chicounity3d.wordpress.com/2014/05/23/how-to-lerp-like-a-pro/
// // float smooth(float t) {
// // 	return t * t * (3 - 2 * t);
// // }
// float smoother(float t) {
// 	return t * t * t * (t * (t * 6 - 15) + 10);
// }
// float easeIn(float t) {
// 	return 1 - cos(t * 3.141592654 * 0.5);
// }
// float easeOut(float t) {
// 	return sin(t * 3.141592654 * 0.5);
// }

