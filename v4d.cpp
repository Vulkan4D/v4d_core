#include "v4d.h"


DEFINE_EXTERN_EVENT_BODY(V4D_CORE_INIT, void*)
DEFINE_EXTERN_EVENT_BODY(V4D_CORE_DESTROY, void*)


std::string v4d::GET_V4D_CORE_BUILD_VERSION() {
	return V4D_VERSION;
}

void v4d::Init() {
	//...
	v4d::event::V4D_CORE_INIT(nullptr);
}

void v4d::Destroy() {
	v4d::event::V4D_CORE_DESTROY(nullptr);
	//...
}
