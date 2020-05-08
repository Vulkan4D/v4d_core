#pragma once
#include <v4d.h>

#ifdef _ENABLE_IMGUI
	#define __V4D_Game_IMGUI ,RunImGui
#else
	#define __V4D_Game_IMGUI
#endif

class V4DLIB V4D_Game {
	V4D_MODULE_CLASS_H(V4D_Game
		,OrderIndex
		,Init
		,LoadScene
		,UnloadScene
		__V4D_Game_IMGUI
		,Update
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Scene&)
	V4D_MODULE_FUNC(void, LoadScene, v4d::graphics::Scene&)
	V4D_MODULE_FUNC(void, UnloadScene, v4d::graphics::Scene&)
	#ifdef _ENABLE_IMGUI
		V4D_MODULE_FUNC(void, RunImGui)
	#endif
	V4D_MODULE_FUNC(void, Update, v4d::graphics::Scene&)
};
