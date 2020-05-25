#pragma once

#include <v4d.h>

namespace v4d::scene {
	
	struct LightSource {
		glm::dvec3 position {0};
		glm::f32 intensity = 0;
		glm::vec3 color {1};
		glm::u32 type = 0;
		glm::u32 attributes = 0;
		glm::f32 radius = 0;
		glm::f32 custom1 = 0;
		
		uint32_t lightOffset = 0;
		glm::vec3 viewSpacePosition {0};
		
		LightSource (
			glm::dvec3 position = {0,0,0},
			glm::f32 intensity = 0,
			glm::vec3 color = {1,1,1},
			glm::u32 type = 0,
			glm::u32 attributes = 0,
			glm::f32 radius = 0,
			glm::f32 custom1 = 0
		) : 
			position(position),
			intensity(intensity),
			color(color),
			type(type),
			attributes(attributes),
			radius(radius),
			custom1(custom1)
 		{}
	};
	
}
