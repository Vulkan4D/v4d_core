#pragma once

#include "Camera.hpp"
#include "LightSource.hpp"
#include "ObjectInstance.hpp"

namespace v4d::graphics {
	struct V4DLIB Scene {
		mutable std::mutex sceneMutex;
		
		Camera camera {};
		// std::map<std::string, LightSource*> lightSources {};
		std::vector<ObjectInstance*> objectInstances {};
		
		ObjectInstance* AddObjectInstance(std::string type = "standard") {
			std::lock_guard lock(sceneMutex);
			return objectInstances.emplace_back(new ObjectInstance(type));
		}
		
		void RemoveObjectInstance(ObjectInstance* obj) {
			std::lock_guard lock(sceneMutex);
			auto objInScene = std::find(objectInstances.begin(), objectInstances.end(), obj);
			if (objInScene != objectInstances.end()) {
				*objInScene = nullptr;
			}
			delete obj;
			std::remove_if(objectInstances.begin(), objectInstances.end(), [](auto* o){return !o;});
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
