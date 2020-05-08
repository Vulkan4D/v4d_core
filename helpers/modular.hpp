#pragma once
#include <v4d.h>

#ifdef _WIN32
	#define __V4D_MODULE_FILE_HANDLER HINSTANCE
	#define __V4D_MODULE_ERR_TYPE DWORD
	#define __V4D_MODULE_FILE_EXT ".dll"
	#define __V4D_MODULE_LOAD(filePath) LoadLibrary(filePath)
	#define __V4D_MODULE_GET_ERROR() GetLastError()
	#define __V4D_MODULE_UNLOAD(handle) FreeLibrary(handle)
	#define __V4D_MODULE_FUNC_LOAD(funcName) *(void **)(&funcName) = (void*)GetProcAddress(_handle, #funcName);
	// QuickFix for windows.. because 'overwrite_existing' option is not working... we get the 'file exists' error... WTF...
	#define __V4D_MODULE_COPY_NEW(filePath) {std::filesystem::remove(filePath); std::filesystem::copy_file(filePath+".new", filePath);}
#else
	#define __V4D_MODULE_FILE_HANDLER void*
	#define __V4D_MODULE_ERR_TYPE char*
	#define __V4D_MODULE_FILE_EXT ".so"
	#define __V4D_MODULE_LOAD(filePath) dlopen(filePath, RTLD_LAZY)
	#define __V4D_MODULE_GET_ERROR() dlerror()
	#define __V4D_MODULE_UNLOAD(handle) dlclose(handle)
	#define __V4D_MODULE_FUNC_LOAD(funcName) *(void **)(&funcName) = dlsym(_handle, #funcName);
	#define __V4D_MODULE_COPY_NEW(filePath) {std::filesystem::copy_file(filePath+".new", filePath, std::filesystem::copy_options::overwrite_existing);}
#endif

// namespace v4d::modular {
// 	struct ModuleID {
// 		unsigned long vendor = 0;
// 		unsigned long module = 0;
// 		ModuleID(unsigned long vendorID, unsigned long moduleID) : vendor(vendorID), module(moduleID) {}
// 		ModuleID(const std::string& str) {
// 			auto _ = str.find('_');
// 			if (_ == std::string::npos)
// 				return;
// 			std::string vendorStr = str.substr(0, _);
// 			std::string moduleStr = str.substr(_+1);
// 			if (vendorStr.length() > 12 || moduleStr.length() > 12 || vendorStr.length() == 0 || moduleStr.length() == 0)
// 				return;
// 			if (std::string(BASE26_UPPER_CHARS).find(vendorStr[0]) == std::string::npos || std::string(BASE26_LOWER_CHARS).find(moduleStr[0]) == std::string::npos)
// 				return;
// 			vendor = v4d::BaseN::DecodeLong(vendorStr, BASE36_UPPER_CHARS);
// 			module = v4d::BaseN::DecodeLong(moduleStr, BASE36_LOWER_CHARS);
// 		}
// 		ModuleID(const char* str) : ModuleID(std::string(str)) {}
// 		operator std::string () const {
// 			return String();
// 		}
// 		std::string String() const {
// 			if (vendor == 0 || module == 0) return "";
// 			return v4d::BaseN::EncodeLong(vendor, BASE36_UPPER_CHARS) + "_" + v4d::BaseN::EncodeLong(module, BASE36_LOWER_CHARS);
// 		}
// 		bool IsValid() const {
// 			ModuleID tmp = String();
// 			return tmp.vendor != 0 && tmp.module != 0;
// 		}
// 	};
// 	struct ModuleClassID {
// 		unsigned long vendor = 0;
// 		unsigned long moduleClass = 0;
// 		ModuleClassID(unsigned long vendorID, unsigned long classID) : vendor(vendorID), moduleClass(classID) {}
// 		ModuleClassID(const std::string& str) {
// 			auto _ = str.find('_');
// 			if (_ == std::string::npos)
// 				return;
// 			std::string vendorStr = str.substr(0, _);
// 			std::string moduleClassStr = str.substr(_+1);
// 			if (vendorStr.length() > 12 || moduleClassStr.length() > 12 || vendorStr.length() == 0 || moduleClassStr.length() == 0)
// 				return;
// 			if (std::string(BASE26_UPPER_CHARS).find(vendorStr[0]) == std::string::npos || std::string(BASE26_UPPER_CHARS).find(moduleClassStr[0]) == std::string::npos)
// 				return;
// 			moduleClassStr[0] = std::tolower(moduleClassStr[0]);
// 			vendor = v4d::BaseN::DecodeLong(vendorStr, BASE36_UPPER_CHARS);
// 			moduleClass = v4d::BaseN::DecodeLong(moduleClassStr, BASE36_LOWER_CHARS);
// 		}
// 		ModuleClassID(const char* str) : ModuleClassID(std::string(str)) {}
// 		operator std::string () const {
// 			return String();
// 		}
// 		std::string String() const {
// 			if (vendor == 0 || moduleClass == 0) return "";
// 			std::string moduleClassStr = v4d::BaseN::EncodeLong(moduleClass, BASE36_LOWER_CHARS);
// 			moduleClassStr[0] = std::toupper(moduleClassStr[0]);
// 			return v4d::BaseN::EncodeLong(vendor, BASE36_UPPER_CHARS) + "_" + moduleClassStr;
// 		}
// 		bool IsValid() const {
// 			ModuleClassID tmp = String();
// 			return tmp.vendor != 0 && tmp.moduleClass != 0;
// 		}
// 	};
// }

#define V4D_MODULE_CLASS_CPP(moduleClassName)\
	moduleClassName::callback moduleClassName::_modulesLoadCallback = [](moduleClassName*){};\
	moduleClassName::callback moduleClassName::_modulesUnloadCallback = [](moduleClassName*){};\
	moduleClassName::errorCallback moduleClassName::_modulesLoadErrorCallback = [](const char* err){\
		std::cerr << err << std::endl;\
	};\
	std::recursive_mutex moduleClassName::_mutexForLoadedModules {};\
	std::unordered_map<std::string, moduleClassName*> moduleClassName::_loadedModules {};\
	std::unordered_map<std::string, std::vector<moduleClassName*>> moduleClassName::_loadedSortedModules {};\
	void moduleClassName::ModulesSetLoadCallback(const callback&& func) {\
		_modulesLoadCallback = func;\
	}\
	void moduleClassName::ModulesSetUnloadCallback(const callback&& func) {\
		_modulesUnloadCallback = func;\
	}\
	void moduleClassName::ModulesSetLoadErrorCallback(const errorCallback&& func) {\
		_modulesLoadErrorCallback = func;\
	}\
	moduleClassName* moduleClassName::LoadModule(const std::string& mod, bool triggerErrorOnFailure) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		const std::string path = std::string("modules/") + mod + "/" + mod + "." + #moduleClassName;\
		if (_loadedModules.find(mod) == _loadedModules.end()) {\
			moduleClassName* instance = new moduleClassName(path);\
			if (!instance->_handle) {\
				if (triggerErrorOnFailure) {\
					std::ostringstream err;\
					err << "Module '" << path << "' failed to load with error: " << instance->_error;\
					_modulesLoadErrorCallback(err.str().c_str());\
				}\
				delete instance;\
				return nullptr;\
			}\
			_loadedModules.emplace(mod, instance);\
			_modulesLoadCallback(instance);\
		}\
		return _loadedModules[mod];\
	}\
	void moduleClassName::UnloadModule(const std::string& mod) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		moduleClassName* modPtr = nullptr;\
		if (_loadedModules.find(mod) != _loadedModules.end()) {\
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
	void moduleClassName::ForEachModule(const callback&& func) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		for (auto[mod, modulePtr] : _loadedModules) {\
			func(modulePtr);\
		}\
	}\
	void moduleClassName::ForEachSortedModule(const callback&& func, const std::string& sortedListKey) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		for (auto* modulePtr : _loadedSortedModules[sortedListKey]) {\
			func(modulePtr);\
		}\
	}\
	void moduleClassName::SortModules(const std::function<bool(moduleClassName*, moduleClassName*)>& func, const std::string& sortedListKey) {\
		std::lock_guard lock(_mutexForLoadedModules);\
		_loadedSortedModules[sortedListKey].clear();\
		_loadedSortedModules[sortedListKey].reserve(_loadedModules.size());\
		for (auto[mod, modulePtr] : _loadedModules) {\
			_loadedSortedModules[sortedListKey].push_back(modulePtr);\
		}\
		std::sort(_loadedSortedModules[sortedListKey].begin(), _loadedSortedModules[sortedListKey].end(), func);\
	}

#define V4D_MODULE_FUNC(returnType, funcName, ...)\
		returnType (*funcName)(__VA_ARGS__) = nullptr;

#define V4D_MODULE_CLASS_H(moduleClassName, ...)\
	private:\
		using callback = std::function<void(moduleClassName*)>;\
		using errorCallback = std::function<void(const char*)>;\
		static callback _modulesLoadCallback;\
		static callback _modulesUnloadCallback;\
		static errorCallback _modulesLoadErrorCallback;\
		static std::recursive_mutex _mutexForLoadedModules;\
		static std::unordered_map<std::string, moduleClassName*> _loadedModules;\
		static std::unordered_map<std::string, std::vector<moduleClassName*>> _loadedSortedModules;\
	public:\
		static void ModulesSetLoadCallback(const callback&& _modulesLoadCallback);\
		static void ModulesSetUnloadCallback(const callback&& _modulesUnloadCallback);\
		static void ModulesSetLoadErrorCallback(const errorCallback&& _modulesLoadErrorCallback);\
		static moduleClassName* LoadModule(const std::string& mod, bool triggerErrorOnFailure = false);\
		static void UnloadModule(const std::string& mod);\
		static void ForEachModule(const callback&& func);\
		static void ForEachSortedModule(const callback&& func, const std::string& sortedListKey = "");\
		static void SortModules(const std::function<bool(moduleClassName*, moduleClassName*)>& func, const std::string& sortedListKey = "");\
	private:\
		__V4D_MODULE_FILE_HANDLER _handle = 0;\
		__V4D_MODULE_ERR_TYPE _error = 0;\
		std::string _libPath {""};\
	public:\
		moduleClassName(const std::string& filePath) {\
			_libPath = filePath + __V4D_MODULE_FILE_EXT;\
			if (v4d::io::FilePath::FileExists(_libPath+".new")) {\
				__V4D_MODULE_COPY_NEW(_libPath)\
				std::filesystem::remove(filePath+".new");\
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
		DELETE_COPY_MOVE_CONSTRUCTORS(moduleClassName)\
		V4D_MODULE_FUNC(void, ModuleLoad)\
		V4D_MODULE_FUNC(void, ModuleUnload)\
		V4D_MODULE_FUNC(void, ModuleSetCustomPtr, int, void*)\
		V4D_MODULE_FUNC(void*, ModuleGetCustomPtr, int)\
		~moduleClassName() {\
			if (ModuleUnload) ModuleUnload();\
			if (_handle) __V4D_MODULE_UNLOAD(_handle);\
		}