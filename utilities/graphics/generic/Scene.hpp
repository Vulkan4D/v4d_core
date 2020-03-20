#pragma once

#include "Camera.hpp"
#include "LightSource.hpp"
#include "ObjectInstance.hpp"

namespace v4d::graphics {
	struct V4DLIB Scene {
		mutable std::recursive_mutex sceneMutex;
		
		Camera camera {};
		std::vector<ObjectInstance*> objectInstances {};
		
		ObjectInstance* AddObjectInstance(std::string type = "standard") {
			std::lock_guard lock(sceneMutex);
			return objectInstances.emplace_back(new ObjectInstance(type));
		}
		
		void RemoveObjectInstance(ObjectInstance* obj) {
			std::lock_guard lock(sceneMutex);
			if (!obj) return;
			if (obj->GetRayTracingInstanceIndex() != -1) { //TODO maybe replace this with usage count in the future
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
		
		void CollectGarbage() {
			std::lock_guard lock(sceneMutex);
			for (auto* obj : objectInstances) if (obj) {
				if (obj->IsMarkedForDeletion() && obj->GetRayTracingInstanceIndex() == -1) {
					RemoveObjectInstance(obj);
				}
			}
		}
		
		int GetObjectCount() const {
			std::lock_guard lock(sceneMutex);
			return objectInstances.size();
		}
		
		void Lock() const {
			sceneMutex.lock();
		}
		
		void Unlock() const {
			sceneMutex.unlock();
		}
		
	};
}
