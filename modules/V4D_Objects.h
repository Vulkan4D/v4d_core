#pragma once
#include <v4d.h>

class V4DLIB V4D_Objects {
	V4D_MODULE_CLASS_HEADER(V4D_Objects
		,OrderIndex
		,Init
		,CreateObject
		,DestroyObject
		,SendStreamCustomObjectData
		,ReceiveStreamCustomObjectData
		,SendStreamCustomTransformData
		,ReceiveStreamCustomTransformData
		,AddToScene
	)
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init)
	V4D_MODULE_FUNC_DECLARE(void, CreateObject, v4d::scene::NetworkGameObjectPtr obj)
	V4D_MODULE_FUNC_DECLARE(void, DestroyObject, v4d::scene::NetworkGameObjectPtr obj, v4d::scene::Scene* scene)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::WriteOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomObjectData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::ReadOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, SendStreamCustomTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::WriteOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, ReceiveStreamCustomTransformData, v4d::scene::NetworkGameObjectPtr obj, v4d::data::ReadOnlyStream& stream)
	V4D_MODULE_FUNC_DECLARE(void, AddToScene, v4d::scene::NetworkGameObjectPtr obj, v4d::scene::Scene* scene)
};
