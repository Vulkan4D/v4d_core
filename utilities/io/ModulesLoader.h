#pragma once
#include <v4d.h>

#include "SharedLibraryLoader.h"

#define __DEFINE_MODULE_FUNC(returntype, funcName, ...) returntype (*funcName)(__VA_ARGS__);
#ifdef _WINDOWS
	#define __LOAD_MODULE_FUNC(funcName) *(void **)(&funcName) = (void*)GetProcAddress(handle, #funcName);
#else// LINUX
	#define __LOAD_MODULE_FUNC(funcName) *(void **)(&funcName) = dlsym(handle, #funcName);
#endif
#define __LOAD_MODULE_FUNC_REQUIRED(funcName) { \
	__LOAD_MODULE_FUNC(funcName) \
	if (!funcName) { \
		LOG_ERROR("Module '" << name << "' is missing the function " << #funcName); \
		return false; \
	} \
}

namespace v4d::io {
	struct V4DLIB ModuleInstance {
		std::string name;
		std::string path;
		DLL_FILE_HANDLER handle;
		
		std::string moduleName;
		std::string moduleDescription;
		int moduleRevision;
		bool loaded = false;

		// Module Metadata
		__DEFINE_MODULE_FUNC(std::string, __GetCoreBuildVersion)
		__DEFINE_MODULE_FUNC(std::string, __GetModuleName)
		__DEFINE_MODULE_FUNC(std::string, __GetModuleDescription)
		__DEFINE_MODULE_FUNC(int, __GetModuleRevision)
		__DEFINE_MODULE_FUNC(void, __InitModule, v4d::Core*)
		// Predefined optional functions
		__DEFINE_MODULE_FUNC(void, OnLoad)
		__DEFINE_MODULE_FUNC(void, OnDestroy)

		virtual bool __LoadModuleFunctions() {
			// Module Metadata
			__LOAD_MODULE_FUNC_REQUIRED(__GetCoreBuildVersion)
			__LOAD_MODULE_FUNC_REQUIRED(__GetModuleName)
			__LOAD_MODULE_FUNC_REQUIRED(__GetModuleDescription)
			__LOAD_MODULE_FUNC_REQUIRED(__GetModuleRevision)
			__LOAD_MODULE_FUNC_REQUIRED(__InitModule)
			// Predefined optional functions
			__LOAD_MODULE_FUNC(OnLoad)
			__LOAD_MODULE_FUNC(OnDestroy)

			return true;
		}

		ModuleInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core_weak v4dCore);
		virtual ~ModuleInstance();
	};

	class V4DLIB ModulesLoader {
	private:
		SharedLibraryLoader sharedLibraryLoader;
		std::mutex loadedModulesMutex;
		std::unordered_map<std::string, ModuleInstance*> loadedModules;

		std::string GetLibName(const std::string& sysName) const {
			return std::string("V4D_MODULE:") + sysName;
		}

	public:
		v4d_core_weak v4dCore;

		ModulesLoader(v4d_core_weak v4dCore) : v4dCore(v4dCore) {}
		virtual ~ModulesLoader();

		virtual ModuleInstance* Load(const std::string& sysName);
		virtual void Unload(const std::string& sysName);
		virtual void Reload(const std::string& sysName);
	};
}
