#pragma once

#ifdef _V4D_PROJECT

	// Initial source code for the End-Project

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
		if (!v4d::CheckCoreVersion()) return -1; \
		v4d_core v4dCore = std::make_shared<v4d::Core>(); \
		v4dCore->systemsLoader = new v4d::io::SystemsLoader(v4dCore); \
		if (!v4dCore->Init(__VA_ARGS__)) { \
			return -1; \
		}
	
#endif
