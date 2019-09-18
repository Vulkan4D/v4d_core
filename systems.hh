#pragma once

#ifdef _V4D_SYSTEM

	// Initial source code for all V4D Systems (Which will all be compiled as individual dlls loaded at runtime, also used for modding/plugins)

	v4d::Core* v4dCore; //TODO implement some kind of v4dInterface... Because this is somewhat dangerous... 

	V4DSYSTEM std::string __GetCoreBuildVersion() {
		return V4D_VERSION;
	}
	V4DSYSTEM std::string __GetSystemName() {
		return THIS_SYSTEM_NAME;
	}
	V4DSYSTEM int __GetSystemRevision() {
		return THIS_SYSTEM_REVISION;
	}
	V4DSYSTEM std::string __GetSystemDescription() {
		return THIS_SYSTEM_DESCRIPTION;
	}
	V4DSYSTEM void __InitSystem(v4d::Core* instance) {
		v4dCore = instance;
	}

#endif
