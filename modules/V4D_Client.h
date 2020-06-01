#pragma once
#include <v4d.h>

using namespace v4d::networking;

class V4DLIB V4D_Client {
	V4D_MODULE_CLASS_HEADER(V4D_Client
		,OrderIndex
		,Init
		,ReceiveAction
		,ReceiveBurst
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, std::shared_ptr<OutgoingConnection> client, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveAction, v4d::io::Socket&)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveBurst, v4d::io::Socket&)
	
};
