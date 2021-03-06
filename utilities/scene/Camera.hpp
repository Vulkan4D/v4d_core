#pragma once

#include <v4d.h>
#include "utilities/graphics/vulkan/Loader.h"

namespace v4d::scene {
	struct V4DLIB Camera {
		alignas(4) int width = 0;
		alignas(4) int height = 0;
		alignas(4) uint32_t renderOptions = 0;
		alignas(4) uint32_t debugOptions = 0;
		
		alignas(16) glm::vec4 luminance {0}; // Averaged, a = exposure factor
		alignas(32) glm::dvec3 worldPosition {0};
		alignas(8) double fov = 70;
		alignas(32) glm::dvec3 lookDirection {0,1,0};
		alignas(8) double znear = 0.0001; // 0.1 mm
		alignas(32) glm::dvec3 viewUp = {0,0,1};
		alignas(8) double zfar = 1.e19; // 1e19 = 1000 light-years
		alignas(128) glm::dmat4 viewMatrix {1};
		alignas(128) glm::dmat4 projectionMatrix {1};
		alignas(128) glm::dmat4 historyViewMatrix {1};
		alignas(64) glm::mat4 reprojectionMatrix {1};
		alignas(8) glm::vec2 txaaOffset {0};
		alignas(8) glm::vec2 historyTxaaOffset {0};
		
		alignas(4) float brightness = 1.0f;
		alignas(4) float contrast = 1.0f;
		alignas(4) float gamma = 2.2f;
		alignas(4) float time = 0;
		alignas(4) float bounceTimeBudget = 0;
		
		alignas(4) uint32_t renderMode = 1;
		alignas(4) float renderDebugScaling = 1.0f;
		alignas(4) int32_t maxBounces = 5; // -1 = infinite bounces
		alignas(4) uint32_t frameCount = 0;
		alignas(4) int32_t accumulateFrames = -1;
		alignas(4) float denoise = 0;
		
		alignas(32) glm::i64vec3 originOffset {0};
		alignas(32) glm::dvec4 velocity {0}; // w = speed as a ratio of C
		alignas(16) glm::vec3 gravityVector;
		
		float multisamplingKernelSize = 1.0f;
		
		enum : int {
			CAMERA_FRUSTUM_NEAR  = 0,
			CAMERA_FRUSTUM_FAR   = 1,
			CAMERA_FRUSTUM_LEFT  = 2,
			CAMERA_FRUSTUM_RIGHT = 3,
			CAMERA_FRUSTUM_UP    = 4,
			CAMERA_FRUSTUM_DOWN  = 5,
		};
		glm::dvec4 frustumPlanes[6] {};
		
		Camera() {static_assert(sizeof(Camera) < 65536);}
		
		void RefreshProjectionMatrix() {
			// zfar and znear are swapped on purpose. 
			// this technique while also reversing the normal depth test operation will make the depth buffer linear again, giving it a better depth precision on the entire range. 
			projectionMatrix = glm::perspective(glm::radians(fov), (double) width / height, zfar, znear);
			projectionMatrix[1].y *= -1;
			historyViewMatrix = viewMatrix;
		}
		
		void MakeViewMatrix(glm::dvec3 worldPosition, glm::dvec3 lookDirection, glm::dvec3 viewUp) {
			this->worldPosition = worldPosition;
			this->lookDirection = lookDirection;
			this->viewUp = viewUp;
			
			// viewMatrix = glm::lookAt(worldPosition, worldPosition + lookDirection, viewUp);
			// RefreshFrustumPlanes();
			RefreshViewMatrix();
		}
		
		void SetViewMatrix(glm::dmat4 viewMatrix) {
			this->viewMatrix = viewMatrix;
			this->worldPosition = viewMatrix[3];
			RefreshFrustumPlanes();
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
		
		// Frustum culling
		bool IsVisibleInScreen(glm::dvec3 worldPosition, double boundingDistance) {
			for (int i = 0; i < 6; ++i) {
				if (glm::dot(glm::dvec3(frustumPlanes[i]), worldPosition) + frustumPlanes[i].w + boundingDistance <= 0) {
					return false;
				}
			}
			return true;
		}
		
		
		glm::dvec3 GetPositionInScreen(const glm::dvec3& a) const {
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
		
	};
}
