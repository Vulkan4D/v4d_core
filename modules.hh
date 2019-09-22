#pragma once

#ifdef _V4D_MODULE

	// Initial source code for all V4D Modules (Which will all be compiled as individual dlls loaded at runtime, also used for modding/plugins)

	v4d::Core* v4dCore; //TODO implement some kind of v4dInterface... Because this is somewhat dangerous... 

	V4DMODULE std::string __GetCoreBuildVersion() {
		return V4D_VERSION;
	}
	V4DMODULE std::string __GetModuleName() {
		return THIS_MODULE_NAME;
	}
	V4DMODULE int __GetModuleRevision() {
		return THIS_MODULE_REVISION;
	}
	V4DMODULE std::string __GetModuleDescription() {
		return THIS_MODULE_DESCRIPTION;
	}
	V4DMODULE void __InitModule(v4d::Core* instance) {
		v4dCore = instance;
	}

#endif
