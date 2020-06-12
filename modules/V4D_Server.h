#pragma once
#include <v4d.h>

using namespace v4d::networking;

class V4DLIB V4D_Server {
	V4D_MODULE_CLASS_HEADER(V4D_Server
		,OrderIndex
		,Init
		,SlowGameLoop
		,IncomingClient
		,EnqueueAction
		,SendActions
		,SendBursts
		,ReceiveAction
		,ReceiveBurst
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, std::shared_ptr<ListeningServer> server, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(void, SlowGameLoop)
	V4D_MODULE_FUNC_DECLARE(void, IncomingClient, IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, EnqueueAction, v4d::data::WriteOnlyStream& stream, IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, SendActions, v4d::io::SocketPtr stream, IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, SendBursts, v4d::io::SocketPtr stream, IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveAction, v4d::io::SocketPtr stream, IncomingClientPtr client)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveBurst, v4d::io::SocketPtr stream, IncomingClientPtr client)
	
};
