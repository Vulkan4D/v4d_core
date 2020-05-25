#pragma once

#include <v4d.h>

#include "Camera.hpp"
#include "LightSource.hpp"
#include "ObjectInstance.hpp"

namespace v4d::graphics {
	struct V4DLIB Scene {
		mutable std::recursive_mutex sceneMutex;
		
		Camera camera {};
		std::vector<ObjectInstance*> objectInstances {};

		~Scene();
		
		ObjectInstance* AddObjectInstance();
		void RemoveObjectInstance(ObjectInstance* obj);
		
		void CollectGarbage();
		void ClenupObjectInstancesGeometries();
		
		int GetObjectCount() const;
		
		void Lock() const;
		void Unlock() const;
		
		struct V4DLIB RayCastHit {
			v4d::graphics::ObjectInstance* obj = nullptr;
			glm::dvec3 position {0,0,0};
			glm::dvec3 normal {0,0,0};
			RayCastHit();
			RayCastHit(v4d::graphics::ObjectInstance* o, glm::dvec3 p, glm::dvec3 n);
		};
		
		bool RayCastClosest(RayCastHit* hit, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask = 0xffffffff) const;
		bool RayCastClosest(RayCastHit* hit, double minDistance, double maxDistance = -1, uint32_t mask = 0xffffffff) const;
		bool RayCastClosest(RayCastHit* hit, uint32_t mask = 0xffffffff) const;
		
		int RayCastAll(std::vector<RayCastHit>* hits, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask = 0xffffffff) const;
		int RayCastAll(std::vector<RayCastHit>* hits, double minDistance, double maxDistance = -1, uint32_t mask = 0xffffffff) const;
		int RayCastAll(std::vector<RayCastHit>* hits, uint32_t mask = 0xffffffff) const;
	};
}
