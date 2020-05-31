#pragma once
#include <v4d.h>

using namespace v4d::networking;

class V4DLIB V4D_Server {
	V4D_MODULE_CLASS_HEADER(V4D_Server
		,OrderIndex
		,Init
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, std::shared_ptr<ListeningServer> server, v4d::scene::Scene*)
	
};
