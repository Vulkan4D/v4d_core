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

		SharedLibraryInstance(const std::string& name, const std::string& path, const bool& lazyLoad = true) : name(name), path(path) {
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
	
		virtual SharedLibraryInstance* Load(const std::string& name, std::string path = "") {
			std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
			if (path == "") path = name;
			if (loadedLibraries.find(name) == loadedLibraries.end()) {
				SharedLibraryInstance* instance = new SharedLibraryInstance(name, path);
				if (!instance->handle) {
					delete instance;
					return nullptr;
				}
				loadedLibraries.emplace(name, instance);
			}
			return loadedLibraries[name];
		}

		virtual void Unload(const std::string& name) {
			std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
			if (loadedLibraries.find(name) != loadedLibraries.end()) {
				delete loadedLibraries[name];
				loadedLibraries.erase(name);
			}
		}

		virtual void Reload(const std::string& name) {
			std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
			if (loadedLibraries.find(name) != loadedLibraries.end()) {
				std::string path = loadedLibraries[name]->path;
				Unload(name);
				Load(name, path);
			}
		}
		
		virtual ~SharedLibraryLoader() {
			std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
			for (auto lib : loadedLibraries) {
				delete lib.second;
			}
		}
	};
}

#ifdef _WINDOWS
	#define LOAD_DLL_FUNC(instance, returntype, funcName, ...) \
		returntype (*funcName)(__VA_ARGS__); \
		*(void **)(&funcName) = (void*)GetProcAddress(instance->handle, #funcName);
#else// LINUX
	#define LOAD_DLL_FUNC(instance, returntype, funcName, ...) \
		returntype (*funcName)(__VA_ARGS__); \
		*(void **)(&funcName) = dlsym(instance->handle, #funcName);
#endif

#ifdef _WINDOWS
	#define LOAD_DLL_ERR \
		 GetLastError()
#else// LINUX
	#define LOAD_DLL_ERR \
		dlerror()
#endif
