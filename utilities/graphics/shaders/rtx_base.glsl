#if defined(SHADER_RGEN) || defined(SHADER_RMISS) || defined(SHADER_RCHIT) || defined(SHADER_RAHIT) || defined(SHADER_RINT)
	#define RAY_TRACING
	#extension GL_EXT_ray_tracing : enable
	#extension GL_EXT_nonuniform_qualifier : enable
	#extension GL_EXT_buffer_reference : enable
#else
	#define RAY_QUERY
	#extension GL_EXT_ray_query : enable
#endif
