#pragma once
#include <v4d.h>

class V4DLIB V4D_Physics {
	V4D_MODULE_CLASS_H(V4D_Physics
		,OrderIndex
		,Init
		,LoadScene
		,UnloadScene
		,RunUi
		,StepSimulation
		,SlowStepSimulation
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Renderer*, v4d::graphics::Scene*)
	V4D_MODULE_FUNC(void, LoadScene)
	V4D_MODULE_FUNC(void, UnloadScene)
	V4D_MODULE_FUNC(void, RunUi)
	V4D_MODULE_FUNC(void, StepSimulation, double deltaTime)
	V4D_MODULE_FUNC(void, SlowStepSimulation, double deltaTime)
};
