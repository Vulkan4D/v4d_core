#pragma once

#include <v4d.h>

namespace v4d::graphics {
	using namespace glm;
	
	enum LightSourceType : int {
		POINT_LIGHT = 0,
	};
	
	struct LightSource { // used as push constant, so maximum size is 128 bytes
		alignas(32)	dvec3 worldPosition;
		alignas(16)	vec3 color;
		alignas(4)	float intensity;
		alignas(16)	vec3 worldDirection;
		alignas(4)	LightSourceType type;
		alignas(16)	vec3 viewPosition;
		alignas(4)	float innerAngle;
		alignas(16)	vec3 viewDirection;
		alignas(4)	float outerAngle;
		
		// 48 bytes remaining for future use
		
		LightSource(
			LightSourceType type = POINT_LIGHT, 
			dvec3 worldPosition = dvec3{0}, 
			vec3 color = vec3{1}, 
			float intensity = 0, 
			dvec3 worldDirection = dvec3{0},
			float innerAngle = 0, 
			float outerAngle = 0
		) : 
			worldPosition(worldPosition), 
			color(color), 
			intensity(intensity), 
			worldDirection{worldDirection}, 
			type(type),
			viewPosition({0}),
			innerAngle(innerAngle),
			viewDirection({0}),
			outerAngle(max(innerAngle, outerAngle))
		{}
	};
}
