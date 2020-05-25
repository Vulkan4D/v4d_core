#include <v4d.h>

namespace v4d::scene {
	Scene::~Scene() {
		ClearAllRemainingObjects();
	}
	
	ObjectInstancePtr Scene::AddObjectInstance() {
		std::lock_guard lock(sceneMutex);
		auto obj = objectInstances.emplace_back(std::make_shared<ObjectInstance>());
		return obj;
	}
	
	void Scene::RemoveObjectInstance(ObjectInstancePtr obj) {
		std::lock_guard lock(sceneMutex);
		obj->Disable();
		for (auto&[_, func] : objectInstanceRemovedCallbacks) {
			func(obj);
		}
		obj->ClearGeometries();
		int lastIndex = objectInstances.size() - 1;
		auto objInScene = std::find(objectInstances.begin(), objectInstances.end(), obj);
		if (objInScene != objectInstances.end()) {
			if (*objInScene != objectInstances[lastIndex]) {
				*objInScene = objectInstances[lastIndex];
			}
			objectInstances.pop_back();
		} else {
			LOG_WARN("Scene Object to be removed was not present in objectInstances. Object NOT deleted.")
		}
	}
	
	void Scene::ClenupObjectInstancesGeometries() {
		std::lock_guard lock(sceneMutex);
		for (auto obj : objectInstances) {
			for (auto&[_, func] : objectInstanceRemovedCallbacks) {
				func(obj);
			}
			obj->ClearGeometries();
		}
	}
	
	void Scene::ClearAllRemainingObjects() {
		std::lock_guard lock(sceneMutex);
		for (auto obj : objectInstances) {
			RemoveObjectInstance(obj);
		}
		objectInstances.clear();
	}
	
	int Scene::GetObjectCount() const {
		std::lock_guard lock(sceneMutex);
		return objectInstances.size();
	}
	
	void Scene::Lock() const {
		sceneMutex.lock();
	}
	
	void Scene::Unlock() const {
		sceneMutex.unlock();
	}
	
	bool Scene::RayCastClosest(Scene::RayCastHit* hit, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask) const {
		static V4D_Physics* primaryPhysicsModule = V4D_Physics::GetPrimaryModule();
		// if (!primaryPhysicsModule) primaryPhysicsModule = V4D_Physics::GetPrimaryModule();
		if (primaryPhysicsModule && primaryPhysicsModule->RayCastClosest) {
			return primaryPhysicsModule->RayCastClosest(hit, origin, target, mask);
		}
		return false;
	}
	
	Scene::RayCastHit::RayCastHit() {}
	Scene::RayCastHit::RayCastHit(v4d::scene::ObjectInstancePtr o, glm::dvec3 p, glm::dvec3 n) : obj(o), position(p), normal(n) {}
	
	bool Scene::RayCastClosest(Scene::RayCastHit* hit, double minDistance, double maxDistance, uint32_t mask) const {
		return RayCastClosest(hit, camera.worldPosition + camera.lookDirection*minDistance, camera.lookDirection*(maxDistance<0? 1e16 : maxDistance), mask);
	}
	
	bool Scene::RayCastClosest(Scene::RayCastHit* hit, uint32_t mask) const {
		return RayCastClosest(hit, 0, -1, mask);
	}
	
	int Scene::RayCastAll(std::vector<Scene::RayCastHit>* hits, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask) const {
		static V4D_Physics* primaryPhysicsModule = V4D_Physics::GetPrimaryModule();
		// if (!primaryPhysicsModule) primaryPhysicsModule = V4D_Physics::GetPrimaryModule();
		if (primaryPhysicsModule && primaryPhysicsModule->RayCastAll) {
			return primaryPhysicsModule->RayCastAll(hits, origin, target, mask);
		}
		return 0;
	}
	
	int Scene::RayCastAll(std::vector<Scene::RayCastHit>* hits, double minDistance, double maxDistance, uint32_t mask) const {
		return RayCastAll(hits, camera.worldPosition + camera.lookDirection*minDistance, camera.lookDirection*(maxDistance<0? 1e16 : maxDistance), mask);
	}
	
	int Scene::RayCastAll(std::vector<Scene::RayCastHit>* hits, uint32_t mask) const {
		return RayCastAll(hits, 0, -1, mask);
	}
	
}
