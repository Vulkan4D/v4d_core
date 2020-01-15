#pragma once

#ifdef _V4D_PROJECT

	// Initial source code for the Project (App or Game)

	namespace v4d {
		bool CheckCoreVersion() {
			if (V4D_VERSION != v4d::GetCoreBuildVersion()) {
				LOG_ERROR("V4D Core Library version mismatch (PROJECT:" << V4D_VERSION << ", V4D_CORE:" << v4d::GetCoreBuildVersion() << ")")
				return false;
			}
			return true;
		}
	}


	#define V4D_PROJECT_INSTANTIATE_CORE_IN_MAIN(v4dCore, ...) \
		/* Catch Signals */ \
		ATTACH_SIGNAL_HANDLER(V4D_SIGNAL_HANDLER) \
		/* Check Core Library Version */ \
		if (!v4d::CheckCoreVersion()) return -1; \
		/* Instantiate V4D Core */ \
		v4d_core v4dCore = std::make_shared<v4d::Core>(); \
		/* Instantiate ModulesLoader */ \
		v4dCore->modulesLoader = new v4d::modules::ModulesLoader(v4dCore); \
		/* Init V4D Core or Exit on failure to init */ \
		if (!v4dCore->Init(__VA_ARGS__)) { \
			return -1; \
		}
	

#endif
