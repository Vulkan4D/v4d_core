#pragma once

#include "Camera.hpp"
#include "LightSource.hpp"
#include "ObjectInstance.hpp"

namespace v4d::graphics {
	struct V4DLIB Scene {
		Camera camera {};
		// std::map<std::string, LightSource*> lightSources {};
		std::vector<ObjectInstance*> objectInstances {};
	};
}
