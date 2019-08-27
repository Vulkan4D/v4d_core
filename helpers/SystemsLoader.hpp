#pragma once

#include "SharedLibraryLoader.hpp"

#define __DEFINE_SYSTEM_FUNC(returntype, funcName, ...) returntype (*funcName)(__VA_ARGS__);
#ifdef _WINDOWS
	#define __LOAD_SYSTEM_FUNC(funcName) *(void **)(&funcName) = (void*)GetProcAddress(handle, #funcName);
#else// LINUX
	#define __LOAD_SYSTEM_FUNC(funcName) *(void **)(&funcName) = dlsym(handle, #funcName);
#endif
#define __LOAD_SYSTEM_FUNC_REQUIRED(funcName) { \
	__LOAD_SYSTEM_FUNC(funcName) \
	if (!funcName) { \
		LOG_ERROR("System '" << name << "' is missing the function " << #funcName); \
		return false; \
	} \
}

namespace v4d {
	struct SystemInstance : public SharedLibraryInstance {
		std::string systemName;
		std::string systemDescription;
		int systemRevision;
		bool loaded = false;

		// System Metadata
		__DEFINE_SYSTEM_FUNC(std::string, GetSystemName)
		__DEFINE_SYSTEM_FUNC(std::string, GetSystemDescription)
		__DEFINE_SYSTEM_FUNC(int, GetSystemRevision)
		// Predefined optional functions
		__DEFINE_SYSTEM_FUNC(void, OnSystemLoad)
		__DEFINE_SYSTEM_FUNC(void, OnSystemDestroy)

		bool __LoadSystemFunctions() {
			// System Metadata
			__LOAD_SYSTEM_FUNC_REQUIRED(GetSystemName)
			__LOAD_SYSTEM_FUNC_REQUIRED(GetSystemDescription)
			__LOAD_SYSTEM_FUNC_REQUIRED(GetSystemRevision)
			// Predefined optional functions
			__LOAD_SYSTEM_FUNC(OnSystemLoad)
			__LOAD_SYSTEM_FUNC(OnSystemDestroy)

			return true;
		}

		SystemInstance(SharedLibraryInstance* libInstance, const std::string& sysName) {
			name = sysName;
			path = libInstance->path;
			handle = libInstance->handle;
			loaded = __LoadSystemFunctions();
			if (loaded) {
				systemName = GetSystemName();
				systemDescription = GetSystemDescription();
				systemRevision = GetSystemRevision();
				if (OnSystemLoad) OnSystemLoad();
			}
		}
		~SystemInstance() {
			if (loaded && OnSystemDestroy) OnSystemDestroy();
		}
	};

	class SystemsLoader : public SharedLibraryLoader {
	private:
		std::mutex loadedSystemsMutex;
		std::map<std::string, SystemInstance*> loadedSystems;

		inline std::string GetLibName(const std::string& sysName) const {
			return std::string("V4D_SYSTEM:") + sysName;
		}

	public:

		SystemInstance* Load(const std::string& sysName) {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			SystemInstance* systemInstance;
			std::string libName = GetLibName(sysName);
			if (loadedSystems.find(sysName) == loadedSystems.end()) {
				auto libInstance = SharedLibraryLoader::Load(libName, std::string("systems/") + sysName + "/" + sysName);
				if (!libInstance) {
					return nullptr;
				}
				
				LOAD_DLL_FUNC(libInstance, std::string, GET_V4D_SYSTEM_BUILD_VERSION)
				if (!GET_V4D_SYSTEM_BUILD_VERSION) {
					LOG_ERROR("Error getting symbol pointer for GET_V4D_SYSTEM_BUILD_VERSION. " << LOAD_DLL_ERR)
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				std::string systemV4dVersion = GET_V4D_SYSTEM_BUILD_VERSION();
				if (systemV4dVersion != V4D_VERSION) {
					LOG_ERROR("V4D Core Libs version mismatch (App:" << V4D_VERSION << " != System:" << systemV4dVersion << ")")
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				systemInstance = new SystemInstance(libInstance, sysName);
				if (!systemInstance->loaded) {
					delete systemInstance;
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				loadedSystems.emplace(sysName, systemInstance);
			}
			return systemInstance;
		}

		void Unload(const std::string& sysName) override {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			if (loadedSystems.find(sysName) != loadedSystems.end()) {
				delete loadedSystems[sysName];
				loadedSystems.erase(sysName);
				SharedLibraryLoader::Unload(GetLibName(sysName));
			}
		}

		void Reload(const std::string& sysName) override {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			if (loadedSystems.find(sysName) != loadedSystems.end()) {
				Unload(sysName);
				Load(sysName);
			}
		}
		
		virtual ~SystemsLoader() {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			for (auto sys : loadedSystems) {
				delete sys.second;
			}
		}
	};
}
