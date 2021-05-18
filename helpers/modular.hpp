#pragma once

#include "helpers/STREAMABLE.hpp"

namespace v4d::modular {
	struct ModuleID {
		uint64_t vendor;
		uint64_t module;
		
		ModuleID() : vendor(0), module(0) {}
		ModuleID(uint64_t vendorID, uint64_t moduleID) : vendor(vendorID), module(moduleID) {}
		ModuleID(const std::string& str) {
			auto _ = str.find('_');
			if (_ == std::string::npos)
				return;
			std::string vendorStr = str.substr(0, _);
			std::string moduleStr = str.substr(_+1);
			if (vendorStr.length() > 12 || moduleStr.length() > 12 || vendorStr.length() == 0 || moduleStr.length() == 0)
				return;
			if (std::string(BASE26_UPPER_CHARS).find(vendorStr[0]) == std::string::npos || std::string(BASE26_LOWER_CHARS).find(moduleStr[0]) == std::string::npos)
				return;
			vendor = v4d::BaseN::EncodeStringToUInt64(vendorStr, BASE36_UPPER_CHARS);
			module = v4d::BaseN::EncodeStringToUInt64(moduleStr, BASE36_LOWER_CHARS);
		}
		ModuleID(const char* str) : ModuleID(std::string(str)) {}
		
		operator std::string () const {
			return String();
		}
		operator const char* () const {
			return String().c_str();
		}
		std::string String() const {
			if (vendor == 0 || module == 0) return "";
			return v4d::BaseN::DecodeStringFromUInt64(vendor, BASE36_UPPER_CHARS) + "_" + v4d::BaseN::DecodeStringFromUInt64(module, BASE36_LOWER_CHARS);
		}
		bool IsValid() const {
			ModuleID tmp = String();
			return tmp.vendor != 0 && tmp.module != 0;
		}
		
		STREAMABLE(ModuleID, vendor, module)
	};
	struct ModuleClassID {
		uint64_t vendor = 0;
		uint64_t moduleClass = 0;
		
		ModuleClassID() : vendor(0), moduleClass(0) {}
		ModuleClassID(uint64_t vendorID, uint64_t classID) : vendor(vendorID), moduleClass(classID) {}
		ModuleClassID(const std::string& str) {
			auto _ = str.find('_');
			if (_ == std::string::npos)
				return;
			std::string vendorStr = str.substr(0, _);
			std::string moduleClassStr = str.substr(_+1);
			if (vendorStr.length() > 12 || moduleClassStr.length() > 12 || vendorStr.length() == 0 || moduleClassStr.length() == 0)
				return;
			if (std::string(BASE26_UPPER_CHARS).find(vendorStr[0]) == std::string::npos || std::string(BASE26_UPPER_CHARS).find(moduleClassStr[0]) == std::string::npos)
				return;
			moduleClassStr[0] = std::tolower(moduleClassStr[0]);
			vendor = v4d::BaseN::EncodeStringToUInt64(vendorStr, BASE36_UPPER_CHARS);
			moduleClass = v4d::BaseN::EncodeStringToUInt64(moduleClassStr, BASE36_LOWER_CHARS);
		}
		ModuleClassID(const char* str) : ModuleClassID(std::string(str)) {}
		
		operator std::string () const {
			return String();
		}
		std::string String() const {
			if (vendor == 0 || moduleClass == 0) return "";
			std::string moduleClassStr = v4d::BaseN::DecodeStringFromUInt64(moduleClass, BASE36_LOWER_CHARS);
			moduleClassStr[0] = std::toupper(moduleClassStr[0]);
			return v4d::BaseN::DecodeStringFromUInt64(vendor, BASE36_UPPER_CHARS) + "_" + moduleClassStr;
		}
		bool IsValid() const {
			ModuleClassID tmp = String();
			return tmp.vendor != 0 && tmp.moduleClass != 0;
		}
		
		STREAMABLE(ModuleClassID, vendor, moduleClass)
	};
}

#define V4D_MODULE_CLASS_CPP(ModuleClassName)\
	ModuleClassName::callbackByPtr ModuleClassName::_modulesLoadCallback = [](ModuleClassName*){};\
	ModuleClassName::callbackByPtr ModuleClassName::_modulesUnloadCallback = [](ModuleClassName*){};\
	ModuleClassName::errorCallback ModuleClassName::_modulesLoadErrorCallback = [](const char* err){\
		std::cerr << err << std::endl;\
	};\
	std::recursive_mutex ModuleClassName::_mutexForLoadedModules {};\
	std::unordered_map<std::string, ModuleClassName*> ModuleClassName::_loadedModules {};\
	std::unordered_map<std::string, std::vector<ModuleClassName*>> ModuleClassName::_loadedSortedModules {};\
	void ModuleClassName::ModulesSetLoadCallback(const callbackByPtr&& func) {\
		_modulesLoadCallback = func;\
	}\
	void ModuleClassName::ModulesSetUnloadCallback(const callbackByPtr&& func) {\
		_modulesUnloadCallback = func;\
	}\
	void ModuleClassName::ModulesSetLoadErrorCallback(const errorCallback&& func) {\
		_modulesLoadErrorCallback = func;\
	}\
	ModuleClassName* ModuleClassName::LoadModule(const std::string& mod, bool triggerErrorOnFailure) {\
		const std::string filePath = std::string("modules/") + mod + "/" + mod + "." + #ModuleClassName;\
		return LoadModuleLib<ModuleClassName>(mod, triggerErrorOnFailure, filePath);\
	}\
	ModuleClassName* ModuleClassName::GetModule(const std::string& mod) {\
		if (_loadedModules.contains(mod)) {\
			return _loadedModules.at(mod);\
		}\
		return nullptr;\
	}\
	void ModuleClassName::UnloadModule(const std::string& mod) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		ModuleClassName* modPtr = nullptr;\
		if (_loadedModules.contains(mod)) {\
			_modulesUnloadCallback(_loadedModules[mod]);\
			modPtr = _loadedModules[mod];\
			delete modPtr;\
			_loadedModules.erase(mod);\
		}\
		if (modPtr) for (auto&[key, modulesList] : _loadedSortedModules) {\
			if (auto it = std::find(modulesList.begin(), modulesList.end(), modPtr); it != modulesList.end()) {\
				modulesList.erase(it);\
			}\
		}\
	}\
	size_t ModuleClassName::Count() {\
		std::lock_guard lock(_mutexForLoadedModules);\
		return _loadedModules.size();\
	}\
	void ModuleClassName::ForEachModule(const callbackByPtrRef&& func) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		for (auto[mod, modulePtr] : _loadedModules) {\
			func(modulePtr);\
			if (!modulePtr) break;\
		}\
	}\
	void ModuleClassName::ForEachSortedModule(const callbackByPtrRef&& func, const std::string& sortedListKey) {\
		for (auto* modulePtr : _loadedSortedModules[sortedListKey]) {\
			func(modulePtr);\
			if (!modulePtr) break;\
		}\
	}\
	void ModuleClassName::SortModules(const std::function<bool(ModuleClassName*, ModuleClassName*)>& func, const std::string& sortedListKey) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		_loadedSortedModules[sortedListKey].clear();\
		_loadedSortedModules[sortedListKey].reserve(_loadedModules.size());\
		for (auto[mod, modulePtr] : _loadedModules) {\
			_loadedSortedModules[sortedListKey].push_back(modulePtr);\
		}\
		std::sort(_loadedSortedModules[sortedListKey].begin(), _loadedSortedModules[sortedListKey].end(), func);\
	}\
	void ModuleClassName::UnloadModules() {\
		std::lock_guard lock(_mutexForLoadedModules);\
		for (auto[mod, modulePtr] : _loadedModules) {\
			delete modulePtr;\
		}\
		_loadedModules.clear();\
		_loadedSortedModules.clear();\
	}\
	ModuleClassName::~ModuleClassName() {\
		if (ModuleUnload) ModuleUnload();\
		if (_handle) __V4D_MODULE_UNLOAD(_handle);\
	}

#ifdef _WIN32
	#define __V4D_MODULE_FILE_HANDLER HINSTANCE
	#define __V4D_MODULE_ERR_TYPE DWORD
	#define __V4D_MODULE_FILE_EXT ".dll"
	#define __V4D_MODULE_LOAD(filePath) LoadLibrary(filePath)
	#define __V4D_MODULE_GET_ERROR() GetLastError()
	#define __V4D_MODULE_UNLOAD(handle) FreeLibrary(handle)
	#ifdef _V4D_IN_EDITOR // This is just for ease of editor validation, using a fake class method overrider instead of a function pointer definition
		#define __V4D_MODULE_FUNC_LOAD(FuncName)
	#else
		#define __V4D_MODULE_FUNC_LOAD(FuncName) *(void **)(&FuncName) = (void*)GetProcAddress(_handle, #FuncName);
	#endif
	// QuickFix for windows.. because 'overwrite_existing' option is not working... we get the 'file exists' error... WTF...
	#define __V4D_MODULE_COPY_NEW(filePath) {std::filesystem::remove(filePath); std::filesystem::copy_file(filePath+".new", filePath);}
#else
	#define __V4D_MODULE_FILE_HANDLER void*
	#define __V4D_MODULE_ERR_TYPE char*
	#define __V4D_MODULE_FILE_EXT ".so"
	#define __V4D_MODULE_LOAD(filePath) dlopen(filePath, RTLD_LAZY)
	#define __V4D_MODULE_GET_ERROR() dlerror()
	#define __V4D_MODULE_UNLOAD(handle) dlclose(handle)
	#ifdef _V4D_IN_EDITOR // This is just for ease of editor validation, using a fake class method overrider instead of a function pointer definition
		#define __V4D_MODULE_FUNC_LOAD(FuncName)
	#else
		#define __V4D_MODULE_FUNC_LOAD(FuncName) *(void **)(&FuncName) = dlsym(_handle, #FuncName);
	#endif
	#define __V4D_MODULE_COPY_NEW(filePath) {std::filesystem::copy_file(filePath+".new", filePath, std::filesystem::copy_options::overwrite_existing);}
#endif

#ifdef _V4D_IN_EDITOR // This is just for ease of editor validation, using a fake class method overrider instead of a function pointer definition
	// Define which class your module extends
	#define V4D_MODULE_CLASS(c) class c ## _MOD : private c
	// Define a function within your module, using the same structure as in the V4D_MODULE_FUNC_DECLARE lines in the module class declaration
	#define V4D_MODULE_FUNC(ReturnType, FuncName, ...) ReturnType FuncName ## _MOD (__VA_ARGS__) override
	#define V4D_MODULE_FUNC_DECLARE(ReturnType, FuncName, ...) private: virtual ReturnType FuncName ## _MOD (__VA_ARGS__); public: ReturnType (*FuncName)(__VA_ARGS__) = nullptr;
#else
	#define V4D_MODULE_CLASS(c) extern "C"
	#define V4D_MODULE_FUNC(ReturnType, FuncName, ...) ReturnType FuncName (__VA_ARGS__)
	#define V4D_MODULE_FUNC_DECLARE(ReturnType, FuncName, ...) public: ReturnType (*FuncName)(__VA_ARGS__) = nullptr;
#endif

#define V4D_MODULE_CLASS_HEADER(ModuleClassName, ...)\
	protected:\
		using callbackByPtr = std::function<void(ModuleClassName*)>;\
		using callbackByPtrRef = std::function<void(ModuleClassName*&)>;\
		using errorCallback = std::function<void(const char*)>;\
		static callbackByPtr _modulesLoadCallback;\
		static callbackByPtr _modulesUnloadCallback;\
		static errorCallback _modulesLoadErrorCallback;\
		static std::recursive_mutex _mutexForLoadedModules;\
		static std::unordered_map<std::string, ModuleClassName*> _loadedModules;\
		static std::unordered_map<std::string, std::vector<ModuleClassName*>> _loadedSortedModules;\
		template<class ModuleClass>\
		static ModuleClass* LoadModuleLib(const std::string& mod, bool triggerErrorOnFailure, std::string filePath) {\
			std::lock_guard lock(_mutexForLoadedModules);\
			if (!_loadedModules.contains(mod)) {\
				ModuleClass* instance = new ModuleClass(filePath);\
				instance->_moduleName = mod;\
				if (!instance->_handle) {\
					if (triggerErrorOnFailure) {\
						std::ostringstream err;\
						err << "Module '" << filePath << "' failed to load with error: " << instance->_error;\
						_modulesLoadErrorCallback(err.str().c_str());\
					}\
					delete instance;\
					return nullptr;\
				}\
				_loadedModules.emplace(mod, instance);\
				_modulesLoadCallback((ModuleClassName*)instance);\
				return instance;\
			}\
			return dynamic_cast<ModuleClass*>(_loadedModules.at(mod));\
		}\
	public:\
		static void ModulesSetLoadCallback(const callbackByPtr&& _modulesLoadCallback);\
		static void ModulesSetUnloadCallback(const callbackByPtr&& _modulesUnloadCallback);\
		static void ModulesSetLoadErrorCallback(const errorCallback&& _modulesLoadErrorCallback);\
		static ModuleClassName* LoadModule(const std::string& mod, bool triggerErrorOnFailure = false);\
		static ModuleClassName* GetModule(const std::string& mod);\
		static void UnloadModule(const std::string& mod);\
		static size_t Count();\
		static void ForEachModule(const callbackByPtrRef&& func);\
		static void ForEachSortedModule(const callbackByPtrRef&& func, const std::string& sortedListKey = "");\
		static void SortModules(const std::function<bool(ModuleClassName*, ModuleClassName*)>& func, const std::string& sortedListKey = "");\
		static void UnloadModules();\
	protected:\
		__V4D_MODULE_FILE_HANDLER _handle = 0;\
		__V4D_MODULE_ERR_TYPE _error = 0;\
		std::string _libPath {""};\
		std::string _moduleName {""};\
	public:\
		__V4D_MODULE_FILE_HANDLER ModuleLibraryHandle() const {return _handle;}\
		std::string ModuleLibraryFilePath() const {return _libPath;}\
		std::string ModuleName() const {return _moduleName;}\
		ModuleClassName(const std::string& filePath) {\
			_libPath = filePath + __V4D_MODULE_FILE_EXT;\
			if (v4d::io::FilePath::FileExists(_libPath+".new")) {\
				__V4D_MODULE_COPY_NEW(_libPath)\
				std::filesystem::remove(_libPath+".new");\
			}\
			_handle = __V4D_MODULE_LOAD(_libPath.c_str());\
			if (!_handle) {_error = __V4D_MODULE_GET_ERROR(); return;}\
			FOR_EACH(__V4D_MODULE_FUNC_LOAD, __VA_ARGS__)\
			__V4D_MODULE_FUNC_LOAD(ModuleLoad)\
			__V4D_MODULE_FUNC_LOAD(ModuleUnload)\
			__V4D_MODULE_FUNC_LOAD(ModuleSetCustomPtr)\
			__V4D_MODULE_FUNC_LOAD(ModuleGetCustomPtr)\
			if (ModuleLoad) ModuleLoad();\
		}\
		DELETE_COPY_MOVE_CONSTRUCTORS(ModuleClassName)\
		V4D_MODULE_FUNC_DECLARE(void, ModuleLoad)\
		V4D_MODULE_FUNC_DECLARE(void, ModuleUnload)\
		V4D_MODULE_FUNC_DECLARE(void, ModuleSetCustomPtr, int, void*)\
		V4D_MODULE_FUNC_DECLARE(void*, ModuleGetCustomPtr, int)\
		virtual ~ModuleClassName();

#define V4D_MODULE_CLASS_DERIVED(DerivedModuleClassName, BaseModuleClassName, ...)\
	protected:\
		using callbackByPtr = std::function<void(DerivedModuleClassName*)>;\
		using callbackByPtrRef = std::function<void(DerivedModuleClassName*&)>;\
	public:\
		static DerivedModuleClassName* LoadModule(const std::string& mod, bool triggerErrorOnFailure = false) {\
			DerivedModuleClassName* derivedModule = LoadModuleLib<DerivedModuleClassName>(mod, triggerErrorOnFailure, std::string("modules/") + mod + "/" + mod + "." + #DerivedModuleClassName);\
			if (derivedModule) return derivedModule;\
			return LoadModuleLib<DerivedModuleClassName>(mod, triggerErrorOnFailure, std::string("modules/") + mod + "/" + mod + "." + #BaseModuleClassName);\
		}\
		static DerivedModuleClassName* GetModule(const std::string& mod) {\
			DerivedModuleClassName* derivedModule = dynamic_cast<DerivedModuleClassName*>(BaseModuleClassName::GetModule(mod));\
			return derivedModule? derivedModule : nullptr;\
		}\
		static void ForEachModule(const callbackByPtrRef&& func) {\
			std::lock_guard lock(_mutexForLoadedModules);\
			for (auto[mod, modulePtr] : _loadedModules) {\
				DerivedModuleClassName* derivedModulePtr = dynamic_cast<DerivedModuleClassName*>(modulePtr);\
				if (derivedModulePtr) func(derivedModulePtr);\
				if (!modulePtr) break;\
			}\
		}\
		static void ForEachSortedModule(const callbackByPtrRef&& func, const std::string& sortedListKey = "") {\
			for (BaseModuleClassName* modulePtr : _loadedSortedModules[sortedListKey]) {\
				DerivedModuleClassName* derivedModulePtr = dynamic_cast<DerivedModuleClassName*>(modulePtr);\
				if (derivedModulePtr) func(derivedModulePtr);\
				if (!modulePtr) break;\
			}\
		}\
		static void SortModules(const std::function<bool(DerivedModuleClassName*, DerivedModuleClassName*)>& func, const std::string& sortedListKey = "") {\
			BaseModuleClassName::SortModules([&func, &sortedListKey](BaseModuleClassName* a, BaseModuleClassName* b) -> bool {\
				DerivedModuleClassName* derivedA = dynamic_cast<DerivedModuleClassName*>(a);\
				DerivedModuleClassName* derivedB = dynamic_cast<DerivedModuleClassName*>(b);\
				if (derivedA && derivedB) return func(derivedA, derivedB);\
				return true;\
			});\
		}\
	public:\
		DerivedModuleClassName(const std::string& filePath) : BaseModuleClassName(filePath) {\
			if (!_handle) return;\
			FOR_EACH(__V4D_MODULE_FUNC_LOAD, __VA_ARGS__)\
		}\
		DELETE_COPY_MOVE_CONSTRUCTORS(DerivedModuleClassName)\
		~DerivedModuleClassName() {}
	
#define V4D_MODULE_ASSET_PATH(moduleName, assetRelativePath) (\
	"modules/" moduleName "/assets/" assetRelativePath \
)

#define V4D_MODULE_DATA_PATH(moduleName, dataRelativePath) (\
	"modules/" moduleName "/data/" dataRelativePath \
)

#define V4D_MODULE_CACHE_PATH(moduleName, cacheRelativePath) (\
	"modules/" moduleName "/cache/" cacheRelativePath \
)

#define V4D_MODULE_ASSET_PATH_STR(moduleName, assetRelativePath) (\
	std::string("modules/") + moduleName + "/assets/" + assetRelativePath \
)

#define V4D_MODULE_DATA_PATH_STR(moduleName, dataRelativePath) (\
	std::string("modules/") + moduleName + "/data/" + dataRelativePath \
)

#define V4D_MODULE_CACHE_PATH_STR(moduleName, cacheRelativePath) (\
	std::string("modules/") + moduleName + "/cache/" + cacheRelativePath \
)
