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
	
	bool Scene::PhysicsRayCastClosest(Scene::RayCastHit* hit, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask) const {
		bool hasHit = false;
		V4D_Mod::ForEachSortedModule([&](auto mod){
			if (mod->PhysicsRayCastClosest) {
				hasHit = mod->PhysicsRayCastClosest(hit, origin, target, mask);
				if (hasHit) mod = nullptr;
			}
		});
		return hasHit;
	}
	
	Scene::RayCastHit::RayCastHit() {}
	Scene::RayCastHit::RayCastHit(v4d::scene::ObjectInstancePtr o, glm::dvec3 p, glm::dvec3 n) : obj(o), position(p), normal(n) {}
	
	bool Scene::PhysicsRayCastClosest(Scene::RayCastHit* hit, double minDistance, double maxDistance, uint32_t mask) const {
		return PhysicsRayCastClosest(hit, camera.worldPosition + camera.lookDirection*minDistance, camera.lookDirection*(maxDistance<0? 1e16 : maxDistance), mask);
	}
	
	bool Scene::PhysicsRayCastClosest(Scene::RayCastHit* hit, uint32_t mask) const {
		return PhysicsRayCastClosest(hit, 0, -1, mask);
	}
	
	int Scene::PhysicsRayCastAll(std::vector<Scene::RayCastHit>* hits, const glm::dvec3& origin, const glm::dvec3& target, uint32_t mask) const {
		int nbHits = 0;
		V4D_Mod::ForEachSortedModule([&](auto mod){
			if (mod->PhysicsRayCastAll) {
				nbHits = mod->PhysicsRayCastAll(hits, origin, target, mask);
			}
		});
		return nbHits;
	}
	
	int Scene::PhysicsRayCastAll(std::vector<Scene::RayCastHit>* hits, double minDistance, double maxDistance, uint32_t mask) const {
		return PhysicsRayCastAll(hits, camera.worldPosition + camera.lookDirection*minDistance, camera.lookDirection*(maxDistance<0? 1e16 : maxDistance), mask);
	}
	
	int Scene::PhysicsRayCastAll(std::vector<Scene::RayCastHit>* hits, uint32_t mask) const {
		return PhysicsRayCastAll(hits, 0, -1, mask);
	}
	
}
