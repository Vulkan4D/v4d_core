#pragma once

#include <v4d.h>

namespace v4d::graphics {
	struct Camera {
		alignas(4) int width = 0;
		alignas(4) int height = 0;
		alignas(32) glm::dvec3 worldPosition {0};
		alignas(8) double fov = 70;
		alignas(32) glm::dvec3 lookDirection {0,1,0};
		alignas(8) double znear = 0.001; // 1 mm
		alignas(32) glm::dvec3 viewUp = {0,0,1};
		alignas(8) double zfar = 1.e16; // 1e16 = 1 light-year
		alignas(128) glm::dmat4 viewMatrix {1};
		alignas(128) glm::dmat4 projectionMatrix {1};
		
		void RefreshViewMatrix() {
			viewMatrix = glm::lookAt(worldPosition, worldPosition + lookDirection, viewUp);
		}
		
		void RefreshProjectionMatrix() {
			// zfar and znear are swapped on purpose. 
			// this technique while also reversing the normal depth test operation will make the depth buffer linear again, giving it a better depth precision on the entire range. 
			projectionMatrix = glm::perspective(glm::radians(fov), (double) width / height, zfar, znear);
			projectionMatrix[1][1] *= -1;
		}
		
		/* // UNUSED FOR NOW...
		glm::dvec3 GetPositionInScreen(glm::dvec3 a) const {
			auto pos = projectionMatrix * viewMatrix * glm::dvec4(a, 1);
			return glm::dvec3{pos.x, pos.y, pos.z} / pos.w;
		}
		double GetApproximateBoundingSizeInScreen(glm::dvec3 a, glm::dvec3 b) const {
			return glm::distance(GetPositionInScreen(a) / 2.0, GetPositionInScreen(b) / 2.0);
		}
		double GetFixedBoundingSizeInScreen(glm::dvec3 a, glm::dvec3 b) const {
			auto v = glm::lookAt(worldPosition, (a + b)/2.0, viewUp);
			auto posA = projectionMatrix * v * glm::dvec4(a, 1);
			posA /= posA.w;
			auto posB = projectionMatrix * v * glm::dvec4(b, 1);
			posB /= posB.w;
			return glm::distance(glm::dvec2(posA.x, posA.y) / 2.0, glm::dvec2(posB.x, posB.y) / 2.0);
		}
		bool IsVisibleInScreen(glm::dvec3 a) const {
			auto pp = GetPositionInScreen(a);
			return (pp.z < 1.0 && glm::abs(pp.x) < 1.0 && glm::abs(pp.y) < 1.0);
		}
		*/
	};
}
