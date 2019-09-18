#pragma once

#include "SharedLibraryLoader.h"

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

namespace v4d::io {
	struct SystemInstance : public SharedLibraryInstance {
		std::string systemName;
		std::string systemDescription;
		int systemRevision;
		bool loaded = false;

		// System Metadata
		__DEFINE_SYSTEM_FUNC(std::string, __GetCoreBuildVersion)
		__DEFINE_SYSTEM_FUNC(std::string, __GetSystemName)
		__DEFINE_SYSTEM_FUNC(std::string, __GetSystemDescription)
		__DEFINE_SYSTEM_FUNC(int, __GetSystemRevision)
		__DEFINE_SYSTEM_FUNC(void, __InitSystem, v4d::Core*)
		// Predefined optional functions
		__DEFINE_SYSTEM_FUNC(void, OnLoad)
		__DEFINE_SYSTEM_FUNC(void, OnDestroy)

		virtual bool __LoadSystemFunctions() {
			// System Metadata
			__LOAD_SYSTEM_FUNC_REQUIRED(__GetCoreBuildVersion)
			__LOAD_SYSTEM_FUNC_REQUIRED(__GetSystemName)
			__LOAD_SYSTEM_FUNC_REQUIRED(__GetSystemDescription)
			__LOAD_SYSTEM_FUNC_REQUIRED(__GetSystemRevision)
			__LOAD_SYSTEM_FUNC_REQUIRED(__InitSystem)
			// Predefined optional functions
			__LOAD_SYSTEM_FUNC(OnLoad)
			__LOAD_SYSTEM_FUNC(OnDestroy)

			return true;
		}

		SystemInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core v4dCore) {
			name = sysName;
			path = libInstance->path;
			handle = libInstance->handle;
			loaded = __LoadSystemFunctions();
			if (loaded) {
				systemName = __GetSystemName();
				systemDescription = __GetSystemDescription();
				systemRevision = __GetSystemRevision();
				__InitSystem(v4dCore.get());
				if (OnLoad) OnLoad();
			}
		}
		~SystemInstance() {
			if (loaded && OnDestroy) OnDestroy();
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
		v4d_core v4dCore;

		SystemsLoader(v4d_core v4dCore) : SharedLibraryLoader(), v4dCore(v4dCore) {}
		
		virtual ~SystemsLoader() {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			for (auto sys : loadedSystems) {
				delete sys.second;
			}
		}

		SystemInstance* Load(const std::string& sysName) {
			std::lock_guard<std::mutex> lock(loadedSystemsMutex);
			if (loadedSystems.find(sysName) == loadedSystems.end()) {
				std::string libName = GetLibName(sysName);
				auto libInstance = SharedLibraryLoader::Load(libName, std::string("systems/") + sysName + "/" + sysName);
				if (!libInstance) {
					return nullptr;
				}
				
				LOAD_DLL_FUNC(libInstance, std::string, __GetCoreBuildVersion)
				if (!__GetCoreBuildVersion) {
					LOG_ERROR("Error getting symbol pointer for __GetCoreBuildVersion. " << LOAD_DLL_ERR)
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				std::string systemV4dVersion = __GetCoreBuildVersion();
				if (systemV4dVersion != V4D_VERSION) {
					LOG_ERROR("V4D Core Libs version mismatch (App:" << V4D_VERSION << " != System:" << systemV4dVersion << ")")
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				SystemInstance* systemInstance = new SystemInstance(libInstance, sysName, v4dCore);
				if (!systemInstance->loaded) {
					delete systemInstance;
					SharedLibraryLoader::Unload(libName);
					return nullptr;
				}

				loadedSystems.emplace(sysName, systemInstance);
				return systemInstance;
			}
			return loadedSystems[sysName];
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

	};
}
