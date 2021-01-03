// #version 460 core (this is the default)

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

vec3 UnpackVec3rgb10FromFloat(float f) {
	uint packed = floatBitsToUint(f);
	vec3 color = vec3(
		float((packed >> 20) & 0x3ff) / 1023.0f,
		float((packed >> 10) & 0x3ff) / 1023.0f,
		float((packed) & 0x3ff) / 1023.0f
	);
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


float GetSphereIntersectionViewDistance(vec3 viewPos, vec3 sphereCenterViewPos, float sphereRadius) {
	float dist = length(sphereCenterViewPos);
	if (dist <= sphereRadius) {
		return 0; // we are inside the sphere
	} else if (sphereRadius > 0.0) {
		vec3 direction = normalize(viewPos);
		vec3 oc = -sphereCenterViewPos;
		if (dot(normalize(oc), direction) < 0) {
			float a = dot(direction, direction);
			float b = 2.0 * dot(oc, direction);
			float c = dot(oc,oc) - sphereRadius*sphereRadius;
			float discriminent = b*b - 4*a*c;
			dist = (-b - sqrt(discriminent)) / (2.0*a);
			if (discriminent >= 0) {
				return dist;
			}
		}
	}
	return -1;
}

bool ProceduralSphereIntersection(vec3 centerPos, float sphereRadius, inout vec3 viewPos, out vec3 normal, inout float dist) {
	float minDistance = dist;
	dist = GetSphereIntersectionViewDistance(viewPos, centerPos, sphereRadius);
	if (dist >= 0) {
		dist = max(dist, minDistance);
		viewPos = normalize(viewPos)*dist;
		normal = normalize(viewPos - centerPos);
		return true;
	}
	dist = minDistance;
	return false;
}


// Ease Functions	https://chicounity3d.wordpress.com/2014/05/23/how-to-lerp-like-a-pro/
// float smooth(float t) {
// 	return t * t * (3 - 2 * t);
// }
// float smoother(float t) {
// 	return t * t * t * (t * (t * 6 - 15) + 10);
// }
// float easeIn(float t) {
// 	return 1 - cos(t * 3.141592654 * 0.5);
// }
// float easeOut(float t) {
// 	return sin(t * 3.141592654 * 0.5);
// }

float linearStep(float low, float high, float value) {
	if (high == low) return (value >= low ? 1 : 0);
	return (value - low) / (high - low);
}

float fade(float low, float high, float value){
	float mid = (low+high)*0.5;
	float range = (high-low)*0.5;
	float x = 1.0 - clamp(abs(mid-value)/range, 0.0, 1.0);
	return smoothstep(0.0, 1.0, x);
}

// vec3 getHeatMap(float intensity){
// 	if (intensity <= 0) return vec3(0);
// 	if (intensity >= 1) return vec3(1);
// 	vec3 blue = vec3(0.0, 0.0, 1.0);
// 	vec3 cyan = vec3(0.0, 1.0, 1.0);
// 	vec3 green = vec3(0.0, 1.0, 0.0);
// 	vec3 yellow = vec3(1.0, 1.0, 0.0);
// 	vec3 red = vec3(1.0, 0.0, 0.0);
// 	vec3 color = (
// 		fade(-0.25, 0.25, intensity)*blue +
// 		fade(0.0, 0.5, intensity)*cyan +
// 		fade(0.25, 0.75, intensity)*green +
// 		fade(0.5, 1.0, intensity)*yellow +
// 		smoothstep(0.75, 1.0, intensity)*red
// 	);
// 	return color;
// }

vec3 heatmap(float t) {
	if (t <= 0) return vec3(0);
	if (t >= 1) return vec3(1);
	const vec3 c[10] = {
		vec3(0.0f / 255.0f,   2.0f / 255.0f,  91.0f / 255.0f),
		vec3(0.0f / 255.0f, 108.0f / 255.0f, 251.0f / 255.0f),
		vec3(0.0f / 255.0f, 221.0f / 255.0f, 221.0f / 255.0f),
		vec3(51.0f / 255.0f, 221.0f / 255.0f,   0.0f / 255.0f),
		vec3(255.0f / 255.0f, 252.0f / 255.0f,   0.0f / 255.0f),
		vec3(255.0f / 255.0f, 180.0f / 255.0f,   0.0f / 255.0f),
		vec3(255.0f / 255.0f, 104.0f / 255.0f,   0.0f / 255.0f),
		vec3(226.0f / 255.0f,  22.0f / 255.0f,   0.0f / 255.0f),
		vec3(191.0f / 255.0f,   0.0f / 255.0f,  83.0f / 255.0f),
		vec3(145.0f / 255.0f,   0.0f / 255.0f,  65.0f / 255.0f)
	};

	const float s = t * 10.0f;

	const int cur = int(s) <= 9 ? int(s) : 9;
	const int prv = cur >= 1 ? cur - 1 : 0;
	const int nxt = cur < 9 ? cur + 1 : 9;

	const float blur = 0.8f;

	const float wc = smoothstep(float(cur) - blur, float(cur) + blur, s) * (1.0f - smoothstep(float(cur + 1) - blur, float(cur + 1) + blur, s));
	const float wp = 1.0f - smoothstep(float(cur) - blur, float(cur) + blur, s);
	const float wn = smoothstep(float(cur + 1) - blur, float(cur + 1) + blur, s);

	const vec3 r = wc * c[cur] + wp * c[prv] + wn * c[nxt];
	return vec3(clamp(r.x, 0.0f, 1.0f), clamp(r.y, 0.0f, 1.0f), clamp(r.z, 0.0f, 1.0f));
}



//////////////////////////////////////
// Random

#extension GL_EXT_control_flow_attributes : require
// Generates a seed for a random number generator from 2 inputs plus a backoff
// https://github.com/nvpro-samples/optix_prime_baking/blob/332a886f1ac46c0b3eea9e89a59593470c755a0e/random.h
// https://github.com/nvpro-samples/vk_raytracing_tutorial_KHR/tree/master/ray_tracing_jitter_cam
// https://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm
uint InitRandomSeed(uint val0, uint val1) {
	uint v0 = val0, v1 = val1, s0 = 0;
	[[unroll]]
	for (uint n = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}
uint RandomInt(inout uint seed) {
	return (seed = 1664525 * seed + 1013904223);
}
float RandomFloat(inout uint seed) {
	return (float(RandomInt(seed) & 0x00FFFFFF) / float(0x01000000));
}
vec2 RandomInUnitDisk(inout uint seed) {
	for (;;) {
		const vec2 p = 2 * vec2(RandomFloat(seed), RandomFloat(seed)) - 1;
		if (dot(p, p) < 1) {
			return p;
		}
	}
}
vec3 RandomInUnitSphere(inout uint seed) {
	for (;;) {
		const vec3 p = 2 * vec3(RandomFloat(seed), RandomFloat(seed), RandomFloat(seed)) - 1;
		if (dot(p, p) < 1) {
			return p;
		}
	}
}
