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
	struct V4DLIB SystemInstance : public SharedLibraryInstance {
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

		SystemInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core_weak v4dCore);
		~SystemInstance();
	};

	class V4DLIB SystemsLoader : public SharedLibraryLoader {
	private:
		std::mutex loadedSystemsMutex;
		std::map<std::string, SystemInstance*> loadedSystems;

		inline std::string GetLibName(const std::string& sysName) const {
			return std::string("V4D_SYSTEM:") + sysName;
		}

	public:
		v4d_core_weak v4dCore;

		SystemsLoader(v4d_core_weak v4dCore) : SharedLibraryLoader(), v4dCore(v4dCore) {}
		virtual ~SystemsLoader();

		SystemInstance* Load(const std::string& sysName);
		void Unload(const std::string& sysName) override;
		void Reload(const std::string& sysName) override;
	};
}
