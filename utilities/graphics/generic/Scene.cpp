#include <v4d.h>

namespace v4d::graphics {
	Scene::~Scene() {
		std::lock_guard lock(sceneMutex);
		for (auto* obj : objectInstances) if (obj) {
			delete obj;
		}
		objectInstances.clear();
	}
	
	ObjectInstance* Scene::AddObjectInstance() {
		std::lock_guard lock(sceneMutex);
		return objectInstances.emplace_back(new ObjectInstance());
	}
	
	void Scene::RemoveObjectInstance(ObjectInstance* obj) {
		std::lock_guard lock(sceneMutex);
		if (!obj) return;
		if (obj->AnyGeometryHasInstanceIndex()) { //TODO maybe replace this with usage count in the future, or smart pointer...
			obj->Disable();
			obj->MarkForDeletion();
		} else {
			int lastIndex = objectInstances.size() - 1;
			auto objInScene = std::find(objectInstances.begin(), objectInstances.end(), obj);
			if (objInScene != objectInstances.end()) {
				delete obj;
				if (*objInScene != objectInstances[lastIndex]) {
					*objInScene = objectInstances[lastIndex];
				}
				objectInstances.pop_back();
			} else {
				LOG_WARN("Scene Object to be removed was not present in objectInstances. Object NOT deleted.")
			}
		}
	}
	
	void Scene::CollectGarbage() {
		std::lock_guard lock(sceneMutex);
		for (auto* obj : objectInstances) if (obj) {
			if (obj->IsMarkedForDeletion() && !obj->AnyGeometryHasInstanceIndex()) {
				RemoveObjectInstance(obj);
			}
		}
	}
	
	void Scene::ClenupObjectInstancesGeometries() {
		std::lock_guard lock(sceneMutex);
		for (auto* obj : objectInstances) if (obj) {
			obj->ClearGeometries();
		}
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
	Scene::RayCastHit::RayCastHit(v4d::graphics::ObjectInstance* o, glm::dvec3 p, glm::dvec3 n) : obj(o), position(p), normal(n) {}
	
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
