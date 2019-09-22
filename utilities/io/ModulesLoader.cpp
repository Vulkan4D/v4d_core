#include <v4d.h>

using namespace v4d::io;

ModuleInstance::ModuleInstance(SharedLibraryInstance* libInstance, const std::string& sysName, v4d_core_weak v4dCore) {
	name = sysName;
	path = libInstance->path;
	handle = libInstance->handle;
	loaded = __LoadModuleFunctions();
	if (loaded) {
		moduleName = __GetModuleName();
		moduleDescription = __GetModuleDescription();
		moduleRevision = __GetModuleRevision();
		__InitModule(v4dCore.lock().get());
		if (OnLoad) OnLoad();
	}
}

ModuleInstance::~ModuleInstance() {
	if (loaded && OnDestroy) OnDestroy();
}

///////////////////////////////////////////////////////////

ModulesLoader::~ModulesLoader() {
	std::lock_guard<std::mutex> lock(loadedModulesMutex);
	for (auto sys : loadedModules) {
		delete sys.second;
	}
}

ModuleInstance* ModulesLoader::Load(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedModulesMutex);
	if (loadedModules.find(sysName) == loadedModules.end()) {
		std::string libName = GetLibName(sysName);
		auto libInstance = sharedLibraryLoader.Load(libName, std::string("modules/") + sysName + "/" + sysName);
		if (!libInstance) {
			return nullptr;
		}
		
		LOAD_DLL_FUNC(libInstance, std::string, __GetCoreBuildVersion)
		if (!__GetCoreBuildVersion) {
			LOG_ERROR("Error getting symbol pointer for __GetCoreBuildVersion. " << LOAD_DLL_ERR)
			sharedLibraryLoader.Unload(libName);
			return nullptr;
		}

		std::string moduleV4dVersion = __GetCoreBuildVersion();
		if (moduleV4dVersion != V4D_VERSION) {
			LOG_ERROR("V4D Core Libs version mismatch (App:" << V4D_VERSION << " != Module:" << moduleV4dVersion << ")")
			sharedLibraryLoader.Unload(libName);
			return nullptr;
		}

		ModuleInstance* moduleInstance = new ModuleInstance(libInstance, sysName, v4dCore);
		if (!moduleInstance->loaded) {
			delete moduleInstance;
			sharedLibraryLoader.Unload(libName);
			return nullptr;
		}

		loadedModules.emplace(sysName, moduleInstance);
		return moduleInstance;
	}
	return loadedModules[sysName];
}

void ModulesLoader::Unload(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedModulesMutex);
	if (loadedModules.find(sysName) != loadedModules.end()) {
		delete loadedModules[sysName];
		loadedModules.erase(sysName);
		sharedLibraryLoader.Unload(GetLibName(sysName));
	}
}

void ModulesLoader::Reload(const std::string& sysName) {
	std::lock_guard<std::mutex> lock(loadedModulesMutex);
	if (loadedModules.find(sysName) != loadedModules.end()) {
		Unload(sysName);
		Load(sysName);
	}
}
