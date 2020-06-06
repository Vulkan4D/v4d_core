#pragma once
#include <v4d.h>

class V4DLIB V4D_Objects {
	V4D_MODULE_CLASS_HEADER(V4D_Objects
		,OrderIndex
		,Init
		,BuildObject
		,SendStreamCustomObjectData
		,SendStreamCustomTransformData
		,ReceiveStreamCustomObjectData
		,ReceiveStreamCustomTransformData
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init)
	V4D_MODULE_FUNC_DECLARE(void, BuildObject, v4d::scene::NetworkGameObjectPtr obj, v4d::scene::Scene* scene)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::io::SocketPtr stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::io::SocketPtr stream)
};
