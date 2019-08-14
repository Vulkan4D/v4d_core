#pragma once

#include "SharedLibraryLoader.hpp"

namespace v4d {
	struct SystemInstance : public SharedLibraryInstance {
		SystemInstance(SharedLibraryInstance* instance, std::string systemName) {
			name = systemName;
			path = instance->path;
			handle = instance->handle;
		}
	};

	class SystemsLoader : public SharedLibraryLoader {
	private:
		std::mutex loadedSystemsMutex;
		std::map<std::string, SystemInstance*> loadedSystems;

	public:
		SystemInstance* LoadSystem(std::string systemName) {
			lock_guard<mutex> lock(loadedSystemsMutex);
			std::string libName = std::string("V4D_SYSTEM:") + systemName;
			if (loadedSystems.find(systemName) == loadedSystems.end()) {
				auto instance = Load(libName, std::string("systems/") + systemName + "/" + systemName);
				if (!instance) {
					return nullptr;
				}
				
				LOAD_DLL_FUNC(instance, std::string, GET_V4D_SYSTEM_BUILD_VERSION)

				std::string systemV4dVersion = GET_V4D_SYSTEM_BUILD_VERSION();
				if (systemV4dVersion != V4D_VERSION) {
					LOG_ERROR("V4D Core Libs version mismatch (App:" << V4D_VERSION << " != System:" << systemV4dVersion << ")")
					Unload(libName);
					return nullptr;
				}

				loadedSystems.emplace(systemName, new SystemInstance(instance, systemName));
			}
			return loadedSystems[systemName];
		}
	};
}
