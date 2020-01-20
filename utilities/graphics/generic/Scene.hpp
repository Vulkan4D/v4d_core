#pragma once

#include "Camera.hpp"
#include "LightSource.hpp"

namespace v4d::graphics {
	struct Scene {
		Camera camera {};
		std::map<std::string, LightSource*> lightSources {};
	};
}
