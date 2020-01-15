#pragma once
#include <v4d.h>

#define __V4D_DEFINE_MODULE_FUNC(returntype, funcName, ...) returntype (*funcName)(__VA_ARGS__);
#ifdef _WINDOWS
	#define __V4D_LOAD_MODULE_FUNC(funcName) *(void **)(&funcName) = (void*)GetProcAddress(handle, #funcName);
#else// LINUX
	#define __V4D_LOAD_MODULE_FUNC(funcName) *(void **)(&funcName) = dlsym(handle, #funcName);
#endif
#define __V4D_LOAD_MODULE_FUNC_REQUIRED(funcName) { \
	__V4D_LOAD_MODULE_FUNC(funcName) \
	if (!funcName) { \
		LOG_ERROR("Module '" << name << "' is missing the function " << #funcName); \
		return false; \
	} \
}

namespace v4d::modules {
	using namespace v4d::io;
	
	typedef uint64_t MODULE_ID_T;
	
	const uint64_t SUBMODULE_TYPE_MISC			= 0x00000000LL << 32;
	const uint64_t SUBMODULE_TYPE_CORE			= 0x00000001LL << 32;
	const uint64_t SUBMODULE_TYPE_PROCESSING	= 0x00000002LL << 32;
	const uint64_t SUBMODULE_TYPE_NETWORKING	= 0x00000004LL << 32;
	const uint64_t SUBMODULE_TYPE_GRAPHICS		= 0x00000008LL << 32;
	const uint64_t SUBMODULE_TYPE_AUDIO			= 0x00000010LL << 32;
	const uint64_t SUBMODULE_TYPE_DATA			= 0x00000020LL << 32;
	const uint64_t SUBMODULE_TYPE_TEST			= 0x00000040LL << 32;
	
	class V4DLIB ModuleInstance {
	public:
		typedef std::unordered_map<uint64_t, std::vector<void*>> V4DSubmodules;
		
		DLL_FILE_HANDLER handle;
		
	protected:
		std::string name;
		std::string path;
		MODULE_ID_T fullModuleID;
		std::string moduleName;
		std::string moduleDescription;
		int moduleRevision;
		bool loaded = false;
		
		V4DSubmodules* submodules = nullptr;
		
		static std::unordered_map<MODULE_ID_T, ModuleInstance*> loadedModules;
		
	public:
		// Module Metadata
		__V4D_DEFINE_MODULE_FUNC(std::string, __V4D_GetCoreBuildVersion)
		__V4D_DEFINE_MODULE_FUNC(int, __V4D_GetModuleVendorID)
		__V4D_DEFINE_MODULE_FUNC(int, __V4D_GetModuleID)
		__V4D_DEFINE_MODULE_FUNC(MODULE_ID_T, __V4D_GetFullModuleID)
		__V4D_DEFINE_MODULE_FUNC(std::string, __V4D_GetModuleName)
		__V4D_DEFINE_MODULE_FUNC(std::string, __V4D_GetModuleDescription)
		__V4D_DEFINE_MODULE_FUNC(int, __V4D_GetModuleRevision)
		__V4D_DEFINE_MODULE_FUNC(void, __V4D_InitModule, v4d::Core*)
		__V4D_DEFINE_MODULE_FUNC(V4DSubmodules*, __V4D_GetSubmodules)
		// Predefined optional functions
		__V4D_DEFINE_MODULE_FUNC(void, V4D_ModuleCreate)
		__V4D_DEFINE_MODULE_FUNC(void, V4D_ModuleDestroy)

	protected:
		virtual bool __V4D_LoadModuleFunctions() {
			// Module Metadata
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetCoreBuildVersion)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetModuleVendorID)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetModuleID)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetFullModuleID)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetModuleName)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetModuleDescription)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetModuleRevision)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_InitModule)
			__V4D_LOAD_MODULE_FUNC_REQUIRED(__V4D_GetSubmodules)
			// Predefined optional functions
			__V4D_LOAD_MODULE_FUNC(V4D_ModuleCreate)
			__V4D_LOAD_MODULE_FUNC(V4D_ModuleDestroy)

			return true;
		}
		
	public:

		ModuleInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core_weak v4dCore);
		virtual ~ModuleInstance();
		
		bool IsLoaded() const;
		virtual std::vector<void*>* operator[] (MODULE_ID_T submoduleID);
		
		static ModuleInstance* Get(MODULE_ID_T moduleID);
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
