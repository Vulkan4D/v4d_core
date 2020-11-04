#pragma once

#include <v4d.h>

namespace v4d::scene {
	struct ObjectInstance;
	typedef std::shared_ptr<ObjectInstance> ObjectInstancePtr;
	typedef std::weak_ptr<ObjectInstance> ObjectInstanceWeakPtr;
}

#include "Camera.hpp"
#include "LightSource.hpp"
#include "ObjectInstance.hpp"

namespace v4d::scene {
	struct V4DLIB Scene {
		mutable std::recursive_mutex sceneMutex;
		
		Camera camera {};
		ObjectInstancePtr cameraParent = nullptr;
		glm::dmat4 cameraOffset {1};
		std::vector<ObjectInstancePtr> objectInstances {};
		
		glm::dvec3 gravityVector {0,0,-9.8};
		
		std::unordered_map<std::string, std::function<void(ObjectInstancePtr)>> objectInstanceRemovedCallbacks {};

		~Scene();
		
		ObjectInstancePtr AddObjectInstance();
		void RemoveObjectInstance(ObjectInstancePtr obj);
		
		void ClenupObjectInstancesGeometries();
		void ClearAllRemainingObjects();
		
		int GetObjectCount() const;
		
		void Lock() const;
		void Unlock() const;
		
		struct V4DLIB RayCastHit {
			v4d::scene::ObjectInstancePtr obj = nullptr;
			glm::dvec3 position {0,0,0};
			glm::dvec3 normal {0,0,0};
			RayCastHit();
			RayCastHit(v4d::scene::ObjectInstancePtr o, glm::dvec3 p, glm::dvec3 n);
		};
		
		bool PhysicsRayCastClosest(RayCastHit* hit, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask = 0xffffffff) const;
		bool PhysicsRayCastClosest(RayCastHit* hit, double minDistance, double maxDistance = -1, uint32_t mask = 0xffffffff) const;
		bool PhysicsRayCastClosest(RayCastHit* hit, uint32_t mask = 0xffffffff) const;
		
		int PhysicsRayCastAll(std::vector<RayCastHit>* hits, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask = 0xffffffff) const;
		int PhysicsRayCastAll(std::vector<RayCastHit>* hits, double minDistance, double maxDistance = -1, uint32_t mask = 0xffffffff) const;
		int PhysicsRayCastAll(std::vector<RayCastHit>* hits, uint32_t mask = 0xffffffff) const;
	};
}
