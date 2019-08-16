#include "v4d.h"

//////////////////////////////////////////////////////////
// V4D global events

DEFINE_CORE_EVENT_BODY(V4D_CORE_INIT, int)
DEFINE_CORE_EVENT_BODY(V4D_CORE_DESTROY, int)


//////////////////////////////////////////////////////////
// V4D global functions

std::string v4d::GET_V4D_CORE_BUILD_VERSION() {
	return V4D_VERSION;
}

void v4d::Init() {
	//...
	v4d::event::V4D_CORE_INIT(0);
}

void v4d::Destroy() {
	v4d::event::V4D_CORE_DESTROY(1);
	//...
}
