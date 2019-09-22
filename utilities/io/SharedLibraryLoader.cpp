#include <v4d.h>

using namespace v4d::io;

SharedLibraryInstance::SharedLibraryInstance(const std::string& name, const std::string& path, const bool& lazyLoad = true) : name(name), path(path) {
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

SharedLibraryInstance::~SharedLibraryInstance() {
	if (handle) {
		#ifdef _WINDOWS
			FreeLibrary(handle);
		#else// LINUX
			dlclose(handle);
		#endif
	}
}

///////////////////////////////////////////////////////////

SharedLibraryInstance* SharedLibraryLoader::Load(const std::string& name, std::string path = "") {
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

SharedLibraryLoader::~SharedLibraryLoader() {
	std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
	for (auto lib : loadedLibraries) {
		delete lib.second;
	}
}

void SharedLibraryLoader::Unload(const std::string& name) {
	std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
	if (loadedLibraries.find(name) != loadedLibraries.end()) {
		delete loadedLibraries[name];
		loadedLibraries.erase(name);
	}
}

void SharedLibraryLoader::Reload(const std::string& name) {
	std::lock_guard<std::mutex> lock(loadedLibrariesMutex);
	if (loadedLibraries.find(name) != loadedLibraries.end()) {
		std::string path = loadedLibraries[name]->path;
		Unload(name);
		Load(name, path);
	}
}
