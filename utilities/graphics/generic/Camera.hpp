#pragma once

#include <v4d.h>

namespace v4d::graphics {
	struct Camera {
		alignas(4) int width = 0;
		alignas(4) int height = 0;
		alignas(4) bool txaa = true;
		alignas(4) bool debug = false;
		alignas(32) glm::dvec3 worldPosition {0};
		alignas(8) double fov = 70;
		alignas(32) glm::dvec3 lookDirection {0,1,0};
		alignas(8) double znear = 0.001; // 1 mm
		alignas(32) glm::dvec3 viewUp = {0,0,1};
		alignas(8) double zfar = 1.e16; // 1e16 = 1 light-year
		alignas(128) glm::dmat4 viewMatrix {1};
		alignas(128) glm::dmat4 projectionMatrix {1};
		alignas(128) glm::dmat4 historyViewMatrix {1};
		alignas(64) glm::mat4 reprojectionMatrix {1};
		alignas(16) glm::vec2 txaaOffset {0};
		alignas(16) glm::vec2 historyTxaaOffset {0};
		
		float txaaKernelSize = 1.0f;
		
		enum : int {
			CAMERA_FRUSTUM_NEAR  = 0,
			CAMERA_FRUSTUM_FAR   = 1,
			CAMERA_FRUSTUM_LEFT  = 2,
			CAMERA_FRUSTUM_RIGHT = 3,
			CAMERA_FRUSTUM_UP    = 4,
			CAMERA_FRUSTUM_DOWN  = 5,
		};
		glm::dvec4 frustumPlanes[6] {};
		
		void RefreshProjectionMatrix() {
			// zfar and znear are swapped on purpose. 
			// this technique while also reversing the normal depth test operation will make the depth buffer linear again, giving it a better depth precision on the entire range. 
			projectionMatrix = glm::perspective(glm::radians(fov), (double) width / height, zfar, znear);
			projectionMatrix[1].y *= -1;
			
			// TXAA 
			if (txaa) {
				static unsigned long frameCount = 0;
				static const glm::dvec2 samples8[8] = {
					glm::dvec2(-7.0, 1.0) / 8.0,
					glm::dvec2(-5.0, -5.0) / 8.0,
					glm::dvec2(-1.0, -3.0) / 8.0,
					glm::dvec2(3.0, -7.0) / 8.0,
					glm::dvec2(5.0, -1.0) / 8.0,
					glm::dvec2(7.0, 7.0) / 8.0,
					glm::dvec2(1.0, 3.0) / 8.0,
					glm::dvec2(-3.0, 5.0) / 8.0
				};
				glm::dvec2 texelSize = 1.0 / glm::dvec2(width, height);
				glm::dvec2 subSample = samples8[frameCount % 8] * texelSize * double(txaaKernelSize);
				projectionMatrix[2].x = subSample.x;
				projectionMatrix[2].y = subSample.y;
				historyTxaaOffset = txaaOffset;
				txaaOffset = subSample / 2.0;
				frameCount++;
				
				reprojectionMatrix = (projectionMatrix * historyViewMatrix) * inverse(projectionMatrix * viewMatrix);
				
				// Save Projection and View matrices from previous frame
				historyViewMatrix = viewMatrix;
			}
			
		}
		
		void MakeViewMatrix(glm::dvec3 worldPosition, glm::dvec3 lookDirection, glm::dvec3 viewUp) {
			this->worldPosition = worldPosition;
			this->lookDirection = lookDirection;
			this->viewUp = viewUp;
			RefreshViewMatrix();
		}
		
		void RefreshViewMatrix() {
			viewMatrix = glm::lookAt(worldPosition, worldPosition + lookDirection, viewUp);
			RefreshFrustumPlanes();
		}
		
		void RefreshFrustumPlanes() {
			glm::dmat4 projView = projectionMatrix * viewMatrix;
			
			frustumPlanes[CAMERA_FRUSTUM_RIGHT] = {
				projView[0].w - projView[0].x,
				projView[1].w - projView[1].x,
				projView[2].w - projView[2].x,
				projView[3].w - projView[3].x
			};
			frustumPlanes[CAMERA_FRUSTUM_LEFT] = {
				projView[0].w + projView[0].x,
				projView[1].w + projView[1].x,
				projView[2].w + projView[2].x,
				projView[3].w + projView[3].x
			};
			frustumPlanes[CAMERA_FRUSTUM_DOWN] = {
				projView[0].w + projView[0].y,
				projView[1].w + projView[1].y,
				projView[2].w + projView[2].y,
				projView[3].w + projView[3].y
			};
			frustumPlanes[CAMERA_FRUSTUM_UP] = {
				projView[0].w - projView[0].y,
				projView[1].w - projView[1].y,
				projView[2].w - projView[2].y,
				projView[3].w - projView[3].y
			};
			frustumPlanes[CAMERA_FRUSTUM_FAR] = {
				projView[0].w - projView[0].z,
				projView[1].w - projView[1].z,
				projView[2].w - projView[2].z,
				projView[3].w - projView[3].z
			};
			frustumPlanes[CAMERA_FRUSTUM_NEAR] = {
				projView[0].w + projView[0].z,
				projView[1].w + projView[1].z,
				projView[2].w + projView[2].z,
				projView[3].w + projView[3].z
			};
			
			for (int i = 0; i < 6; ++i) {
				double flScale = 1.0/glm::length(glm::dvec3(frustumPlanes[i]));
				frustumPlanes[i].x *= flScale;
				frustumPlanes[i].y *= flScale;
				frustumPlanes[i].z *= flScale;
				frustumPlanes[i].w *= flScale;
			}
		}
		
		bool IsVisibleInScreen(glm::dvec3 spherePos, double radius) {
			for (int i = 0; i < 6; ++i) {
				if (glm::dot(glm::dvec3(frustumPlanes[i]), spherePos) + frustumPlanes[i].w + radius <= 0) {
					return false;
				}
			}
			return true;
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
