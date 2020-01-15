#pragma once

#ifdef _V4D_MODULE

	// Max number of vendors : 4 billion
	// Max number of modules per vendor : 65 thousand
	// Max number of submodules per module : 65 thousand

	#define V4D_MODULE_ID(ven,mod) ((((v4d::modules::MODULE_ID_T)ven)<<32) | (((v4d::modules::MODULE_ID_T)mod)<<16))
	
	// Initial source code for all V4D Modules (Which will all be compiled as individual dlls loaded at runtime, also used for modding/plugins)

	v4d::Core* v4dCore; //TODO implement some kind of v4dInterface... Because this is somewhat dangerous... 

	v4d::modules::ModuleInstance::V4DSubmodules v4dSubmodules {};

	V4DMODULE int __V4D_GetModuleVendorID() {
		return THIS_MODULE_VENDOR_ID;
	}
	V4DMODULE int __V4D_GetModuleID() {
		return THIS_MODULE_ID;
	}
	V4DMODULE v4d::modules::MODULE_ID_T __V4D_GetFullModuleID() {
		return V4D_MODULE_ID(THIS_MODULE_VENDOR_ID, THIS_MODULE_ID);
	}
	V4DMODULE std::string __V4D_GetCoreBuildVersion() {
		return V4D_VERSION;
	}
	V4DMODULE std::string __V4D_GetModuleName() {
		return THIS_MODULE_NAME;
	}
	V4DMODULE int __V4D_GetModuleRevision() {
		return THIS_MODULE_REV;
	}
	V4DMODULE std::string __V4D_GetModuleDescription() {
		return THIS_MODULE_DESCRIPTION;
	}
	V4DMODULE void __V4D_InitModule(v4d::Core* instance) {
		v4dCore = instance;
	}
	V4DMODULE v4d::modules::ModuleInstance::V4DSubmodules* __V4D_GetSubmodules() {
		return &v4dSubmodules;
	}

#endif
