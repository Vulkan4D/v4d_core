#include "v4d.h"
#ifdef _V4D_CORE

	//////////////////////////////////////////////////////////
	// V4D global events

	DEFINE_CORE_EVENT_BODY(V4D_CORE_INIT, CoreInitEvent&)
	DEFINE_CORE_EVENT_BODY(V4D_CORE_DESTROY, CoreDestroyEvent&)


	//////////////////////////////////////////////////////////
	// V4D global functions

	const std::string v4d::GET_V4D_CORE_BUILD_VERSION() noexcept {
		return V4D_VERSION;
	}

	void v4d::Init() {
		CoreInitEvent e;
		//...
		v4d::event::V4D_CORE_INIT(e);
	}

	void v4d::Destroy() {
		CoreDestroyEvent e;
		v4d::event::V4D_CORE_DESTROY(e);
		//...
	}

#endif
