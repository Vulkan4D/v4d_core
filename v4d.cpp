#include "v4d.h"
#ifdef _V4D_CORE

	//////////////////////////////////////////////////////////
	// V4D global events

	DEFINE_CORE_EVENT_BODY(V4D_CORE_INIT, CoreInitEvent&)
	DEFINE_CORE_EVENT_BODY(V4D_CORE_DESTROY, CoreDestroyEvent&)


	//////////////////////////////////////////////////////////
	// V4D global functions

	const std::string v4d::GetCoreBuildVersion() noexcept {
		return V4D_VERSION;
	}


	//////////////////////////////////////////////////////////
	// V4D Core Instance

	bool v4d::CoreInstance::Init() {
		CoreInitEvent e;
		//...
		v4d::event::V4D_CORE_INIT(e);
		return true;
	}

	void v4d::CoreInstance::Destroy() {
		CoreDestroyEvent e;
		v4d::event::V4D_CORE_DESTROY(e);
		//...
	}

#endif
