#ifdef __cplusplus
	# include <v4d.h>

	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt

	#define int8_t alignas(1) int8_t
	#define uint8_t alignas(1) uint8_t
	#define int16_t alignas(2) int16_t
	#define uint16_t alignas(2) uint16_t
	#define int32_t alignas(4) int32_t
	#define uint32_t alignas(4) uint32_t
	#define int64_t alignas(8) int64_t
	#define uint64_t alignas(8) uint64_t

	#define float32_t alignas(4) glm::float32_t
	#define float64_t alignas(8) glm::float64_t

	#define i8vec2 alignas(2) glm::i8vec2
	#define u8vec2 alignas(2) glm::u8vec2
	#define i8vec3 alignas(4) glm::i8vec3
	#define u8vec3 alignas(4) glm::u8vec3
	#define i8vec4 alignas(4) glm::i8vec4
	#define u8vec4 alignas(4) glm::u8vec4

	#define i16vec2 alignas(4) glm::i16vec2
	#define u16vec2 alignas(4) glm::u16vec2
	#define i16vec3 alignas(8) glm::i16vec3
	#define u16vec3 alignas(8) glm::u16vec3
	#define i16vec4 alignas(8) glm::i16vec4
	#define u16vec4 alignas(8) glm::u16vec4

	#define f32vec2 alignas(8) glm::f32vec2
	#define i32vec2 alignas(8) glm::i32vec2
	#define u32vec2 alignas(8) glm::u32vec2
	#define f32vec3 alignas(16) glm::f32vec3
	#define i32vec3 alignas(16) glm::i32vec3
	#define u32vec3 alignas(16) glm::u32vec3
	#define f32vec4 alignas(16) glm::f32vec4
	#define i32vec4 alignas(16) glm::i32vec4
	#define u32vec4 alignas(16) glm::u32vec4

	#define f64vec2 alignas(16) glm::f64vec2
	#define i64vec2 alignas(16) glm::i64vec2
	#define u64vec2 alignas(16) glm::u64vec2
	#define f64vec3 alignas(32) glm::f64vec3
	#define i64vec3 alignas(32) glm::i64vec3
	#define u64vec3 alignas(32) glm::u64vec3
	#define f64vec4 alignas(32) glm::f64vec4
	#define i64vec4 alignas(32) glm::i64vec4
	#define u64vec4 alignas(32) glm::u64vec4

	#define f32mat3 alignas(16) glm::f32mat3
	#define f64mat3 alignas(32) glm::f64mat3

	#define f32mat4 alignas(16) glm::f32mat4
	#define f64mat4 alignas(32) glm::f64mat4

	#define STATIC_ASSERT_ALIGNED16_SIZE(T, X) static_assert(sizeof(T) == X && sizeof(T) % 16 == 0);
	#define STATIC_ASSERT_SIZE(T, X) static_assert(sizeof(T) == X);

#endif
