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

float PackColorAsFloat(vec4 color) {
	color *= 255.0;
	uvec4 pack = uvec4(
		clamp(uint(color.r), uint(0), uint(255)),
		clamp(uint(color.g), uint(0), uint(255)),
		clamp(uint(color.b), uint(0), uint(255)),
		clamp(uint(color.a), uint(0), uint(255))
	);
	return uintBitsToFloat((pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a);
}

uint PackColorAsUint(vec4 color) {
	color *= 255.0;
	uvec4 pack = uvec4(
		clamp(uint(color.r), uint(0), uint(255)),
		clamp(uint(color.g), uint(0), uint(255)),
		clamp(uint(color.b), uint(0), uint(255)),
		clamp(uint(color.a), uint(0), uint(255))
	);
	return (pack.r << 24) | (pack.g << 16) | (pack.b << 8) | pack.a;
}

float PackUVasFloat(vec2 uv) {
	uv *= 65535.0;
	uvec2 pack = uvec2(
		clamp(uint(uv.s), uint(0), uint(65535)),
		clamp(uint(uv.t), uint(0), uint(65535))
	);
	return uintBitsToFloat((pack.s << 16) | pack.t);
}

uint PackUVasUint(vec2 uv) {
	uv *= 65535.0;
	uvec2 pack = uvec2(
		clamp(uint(uv.s), uint(0), uint(65535)),
		clamp(uint(uv.t), uint(0), uint(65535))
	);
	return (pack.s << 16) | pack.t;
}

// vec3 UnpackNormal(in vec2 norm) {
// 	vec2 fenc = norm * 4.0 - 2.0;
// 	float f = dot(fenc, fenc);
// 	float g = sqrt(1.0 - f / 4.0);
// 	return vec3(fenc * g, 1.0 - f / 2.0);
// }

vec2 UnpackUVfromFloat(in float uv) {
	uint packed = floatBitsToUint(uv);
	return vec2(
		(packed & 0xffff0000) >> 16,
		(packed & 0x0000ffff) >> 0
	) / 65535.0;
}

vec2 UnpackUVfromUint(in uint uv) {
	return vec2(
		(uv & 0xffff0000) >> 16,
		(uv & 0x0000ffff) >> 0
	) / 65535.0;
}

vec4 UnpackColorFromFloat(in float color) {
	uint packed = floatBitsToUint(color);
	return vec4(
		(packed & 0xff000000) >> 24,
		(packed & 0x00ff0000) >> 16,
		(packed & 0x0000ff00) >> 8,
		(packed & 0x000000ff) >> 0
	) / 255.0;
}

vec4 UnpackColorFromUint(in uint color) {
	return vec4(
		(color & 0xff000000) >> 24,
		(color & 0x00ff0000) >> 16,
		(color & 0x0000ff00) >> 8,
		(color & 0x000000ff) >> 0
	) / 255.0;
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

