#include <v4d.h>

using namespace v4d::io;

SystemInstance::SystemInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core_weak v4dCore) {
	name = sysName;
	path = libInstance->path;
	handle = libInstance->handle;
	loaded = __LoadSystemFunctions();
	if (loaded) {
		systemName = __GetSystemName();
		systemDescription = __GetSystemDescription();
		systemRevision = __GetSystemRevision();
		__InitSystem(v4dCore.lock().get());
		if (OnLoad) OnLoad();
	}
}

SystemInstance::~SystemInstance() {
	if (loaded && OnDestroy) OnDestroy();
}

///////////////////////////////////////////////////////////

SystemsLoader::~SystemsLoader() {
	std::lock_guard<std::mutex> lock(loadedSystemsMutex);
	for (auto sys : loadedSystems) {
		delete sys.second;
	}
}

SystemInstance* SystemsLoader::Load(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedSystemsMutex);
	if (loadedSystems.find(sysName) == loadedSystems.end()) {
		std::string libName = GetLibName(sysName);
		auto libInstance = SharedLibraryLoader::Load(libName, std::string("systems/") + sysName + "/" + sysName);
		if (!libInstance) {
			return nullptr;
		}
		
		LOAD_DLL_FUNC(libInstance, std::string, __GetCoreBuildVersion)
		if (!__GetCoreBuildVersion) {
			LOG_ERROR("Error getting symbol pointer for __GetCoreBuildVersion. " << LOAD_DLL_ERR)
			SharedLibraryLoader::Unload(libName);
			return nullptr;
		}

		std::string systemV4dVersion = __GetCoreBuildVersion();
		if (systemV4dVersion != V4D_VERSION) {
			LOG_ERROR("V4D Core Libs version mismatch (App:" << V4D_VERSION << " != System:" << systemV4dVersion << ")")
			SharedLibraryLoader::Unload(libName);
			return nullptr;
		}

		SystemInstance* systemInstance = new SystemInstance(libInstance, sysName, v4dCore);
		if (!systemInstance->loaded) {
			delete systemInstance;
			SharedLibraryLoader::Unload(libName);
			return nullptr;
		}

		loadedSystems.emplace(sysName, systemInstance);
		return systemInstance;
	}
	return loadedSystems[sysName];
}

void SystemsLoader::Unload(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedSystemsMutex);
	if (loadedSystems.find(sysName) != loadedSystems.end()) {
		delete loadedSystems[sysName];
		loadedSystems.erase(sysName);
		SharedLibraryLoader::Unload(GetLibName(sysName));
	}
}

void SystemsLoader::Reload(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedSystemsMutex);
	if (loadedSystems.find(sysName) != loadedSystems.end()) {
		Unload(sysName);
		Load(sysName);
	}
}
