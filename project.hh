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
		
		bool Init() {
			ATTACH_SIGNAL_HANDLER(V4D_SIGNAL_HANDLER)
			if (!v4d::CheckCoreVersion()) return false;
			return true;
		}
	}
	
	
#endif
