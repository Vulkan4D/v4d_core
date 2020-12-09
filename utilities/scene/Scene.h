#pragma once

#include <v4d.h>

#include "Camera.hpp"

namespace v4d::scene {
	struct V4DLIB Scene {
		Camera camera {};
		std::weak_ptr<v4d::graphics::RenderableGeometryEntity> cameraParent;
		glm::dmat4 cameraOffset {1};
		glm::dvec3 gravityVector {0,0,-9.8};
	};
}
