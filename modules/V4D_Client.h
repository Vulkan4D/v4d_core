#pragma once
#include <v4d.h>

using namespace v4d::networking;

class V4DLIB V4D_Client {
	V4D_MODULE_CLASS_HEADER(V4D_Client
		,OrderIndex
		,Init
		,SlowGameLoop
		,EnqueueAction
		,SendActions
		,SendBursts
		,ReceiveAction
		,ReceiveBurst
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, std::shared_ptr<OutgoingConnection> client, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, SlowGameLoop)
	V4D_MODULE_FUNC_DECLARE(void, EnqueueAction, v4d::data::WriteOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, SendActions, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, SendBursts, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveAction, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveBurst, v4d::io::SocketPtr stream)
};
