#pragma once

#ifdef _WINDOWS
	#define DLL_FILE_HANDLER HINSTANCE
#else
	#define DLL_FILE_HANDLER void*
#endif

namespace v4d::io {
	struct V4DLIB SharedLibraryInstance {
		std::string name;
		std::string path;
		DLL_FILE_HANDLER handle;

		SharedLibraryInstance(){}
		SharedLibraryInstance(const std::string& name, const std::string& path, const bool& lazyLoad);
		virtual ~SharedLibraryInstance();
	};

	class V4DLIB SharedLibraryLoader {
	private:
		std::mutex loadedLibrariesMutex;
		std::map<std::string, SharedLibraryInstance*> loadedLibraries;

	public:
	
		virtual SharedLibraryInstance* Load(const std::string& name, std::string path);
		virtual void Unload(const std::string& name);
		virtual void Reload(const std::string& name);
		virtual ~SharedLibraryLoader();
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
