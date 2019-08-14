#pragma once

#ifdef _WINDOWS
	#define DLL_FILE_HANDLER HINSTANCE
#else
	#define DLL_FILE_HANDLER void*
#endif

namespace v4d {
	struct SharedLibraryInstance {
		std::string name;
		std::string path;
		DLL_FILE_HANDLER handle;

		SharedLibraryInstance(const std::string name, const std::string path, bool lazyLoad = true) : name(name), path(path) {
			#ifdef _WINDOWS
				this->path += ".dll";
				this->handle = LoadLibrary(this->path.c_str());
				auto err = GetLastError();
			#else// LINUX
				this->path += ".so";
				this->handle = dlopen(this->path.c_str(), lazyLoad? RTLD_LAZY : RTLD_NOW);
				auto err = dlerror();
			#endif
			if (!handle) {
				LOG_ERROR("Error loading shared library " << name << " in '" << path << "' : " << err)
			}
		}

		SharedLibraryInstance(){}

		~SharedLibraryInstance() {
			if (handle) {
				#ifdef _WINDOWS
					FreeLibrary(handle);
				#else// LINUX
					dlclose(handle);
				#endif
			}
		}
	};

	class SharedLibraryLoader {
	private:
		std::mutex loadedLibrariesMutex;
		std::map<std::string, SharedLibraryInstance*> loadedLibraries;

	public:
	
		SharedLibraryInstance* Load(std::string name, std::string path = "", bool lazyLoad = true) {
			lock_guard<mutex> lock(loadedLibrariesMutex);
			if (path == "") path = name;
			if (loadedLibraries.find(name) == loadedLibraries.end()) {
				SharedLibraryInstance* instance = new SharedLibraryInstance(name, path, lazyLoad);
				if (!instance->handle) {
					delete instance;
					return nullptr;
				}
				loadedLibraries.emplace(name, instance);
			}
			return loadedLibraries[name];
		}

		void Unload(std::string name) {
			lock_guard<mutex> lock(loadedLibrariesMutex);
			if (loadedLibraries.find(name) != loadedLibraries.end()) {
				delete loadedLibraries[name];
				loadedLibraries.erase(name);
			}
		}

		void Reload(std::string name, bool lazyLoad = true) {
			lock_guard<mutex> lock(loadedLibrariesMutex);
			if (loadedLibraries.find(name) != loadedLibraries.end()) {
				std::string path = loadedLibraries[name]->path;
				Unload(name);
				Load(name, path, lazyLoad);
			}
		}
		
	};
}

#ifdef _WINDOWS
	#define LOAD_DLL_FUNC(instance, returntype, funcName, ...) \
		returntype (*funcName)(__VA_ARGS__); \
		*(void **)(&funcName) = (void*)GetProcAddress(instance->handle, #funcName); \
		{ \
			auto err = GetLastError(); \
			if (!funcName) { \
				LOG_ERROR("Error getting symbol pointer for '" << #funcName << "' in shared library '" << instance->name << "' : " << err) \
			} \
		}
#else// LINUX
	#define LOAD_DLL_FUNC(instance, returntype, funcName, ...) \
		returntype (*funcName)(__VA_ARGS__); \
		*(void **)(&funcName) = dlsym(instance->handle, #funcName); \
		{ \
			char* err = dlerror(); \
			if (err != NULL) { \
				LOG_ERROR("Error getting symbol pointer for '" << #funcName << "' in shared library '" << instance->name << "' : " << err) \
			} \
		}
#endif
