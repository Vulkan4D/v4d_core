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

#define V4D_MODULE_CLASS_BEGIN(moduleClassName, ...)\
	class moduleClassName {\
		__V4D_MODULE_FILE_HANDLER _handle = 0;\
		__V4D_MODULE_ERR_TYPE _error = 0;\
		std::string _libPath {""};\
	public:\
		class Modules {\
			using callback = std::function<void(moduleClassName*)>;\
			using errorCallback = std::function<void(const char*)>;\
			auto& LoadCallback() const {static callback loadCallback = [](moduleClassName*){}; return loadCallback;}\
			auto& UnloadCallback() const {static callback unloadCallback = [](moduleClassName*){}; return unloadCallback;}\
			auto& LoadErrorCallback() const {static errorCallback loadErrorCallback = [](const char*){}; return loadErrorCallback;}\
			auto& LoadedModulesMutex() const {static std::recursive_mutex loadedModulesMutex; return loadedModulesMutex;}\
			auto& LoadedModules() const {static std::unordered_map<std::string, moduleClassName*> loadedModules {}; return loadedModules;}\
		public:\
			void SetLoadCallback(const callback&& loadCallback) {LoadCallback() = loadCallback;}\
			void SetUnloadCallback(const callback&& unloadCallback) {UnloadCallback() = unloadCallback;}\
			void SetLoadErrorCallback(const errorCallback&& loadErrorCallback) {LoadErrorCallback() = loadErrorCallback;}\
			moduleClassName* Load(const std::string& mod) {\
				std::lock_guard lock(LoadedModulesMutex());\
				const std::string path = std::string("modules/") + mod + "/" + mod + "." + #moduleClassName;\
				if (LoadedModules().find(mod) == LoadedModules().end()) {\
					moduleClassName* instance = new moduleClassName(path);\
					if (!instance->_handle) {\
						std::ostringstream err;\
						err << "Module '" << path << "' : " << instance->_error;\
						LoadErrorCallback()(err.str().c_str());\
						delete instance;\
						return nullptr;\
					}\
					LoadedModules().emplace(mod, instance);\
					LoadCallback()(instance);\
				}\
				return LoadedModules()[mod];\
			}\
			void Unload(const std::string& mod) {\
				std::lock_guard lock(LoadedModulesMutex());\
				if (LoadedModules().find(mod) != LoadedModules().end()) {\
					UnloadCallback()(LoadedModules()[mod]);\
					delete LoadedModules()[mod];\
					LoadedModules().erase(mod);\
				}\
			}\
			void ForEach(const callback&& func) const {\
				std::lock_guard lock(LoadedModulesMutex());\
				for (auto[mod, modulePtr] : LoadedModules()) {\
					func(modulePtr);\
				}\
			}\
		};\
		moduleClassName(const std::string& filePath) {\
			_libPath = filePath + __V4D_MODULE_FILE_EXT;\
			if (v4d::io::FilePath::FileExists(_libPath+".new")) {\
				__V4D_MODULE_COPY_NEW(_libPath)\
				std::filesystem::remove(filePath+".new");\
			}\
			_handle = __V4D_MODULE_LOAD(_libPath.c_str());\
			if (!_handle) {_error = __V4D_MODULE_GET_ERROR(); return;}\
			FOR_EACH(__V4D_MODULE_FUNC_LOAD, __VA_ARGS__)\
		}\
		DELETE_COPY_MOVE_CONSTRUCTORS(moduleClassName)\
		~moduleClassName() {\
			if (_handle) __V4D_MODULE_UNLOAD(_handle);\
		}
#define V4D_MODULE_FUNC(returnType, funcName, ...)\
		returnType (*funcName)(__VA_ARGS__) = nullptr;
#define V4D_MODULE_CLASS_END() \
	};
