#ifndef __cplusplus
	#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
	#define STATIC_ASSERT_ALIGNED16_SIZE(T,X)
	#define STATIC_ASSERT_SIZE(T,X)
	#define VkDeviceAddress uint64_t
	#define BUFFER_REFERENCE(align) layout(buffer_reference, std430, buffer_reference_align = align) buffer
#endif
