#pragma once
#include <v4d.h>

class V4DLIB V4D_Physics {
	V4D_MODULE_CLASS_H(V4D_Physics
		,OrderIndex
		,Init
		,LoadScene
		,UnloadScene
		,RunUi
		,SlowStepSimulation
		,StepSimulation
		,RayCastClosest
		,RayCastAll
	)
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Renderer*, v4d::scene::Scene*)
	V4D_MODULE_FUNC(void, LoadScene)
	V4D_MODULE_FUNC(void, UnloadScene)
	V4D_MODULE_FUNC(void, RunUi)
	V4D_MODULE_FUNC(void, SlowStepSimulation, double deltaTime)
	
	// If a primary module is defined with this function implemented, this is Only executed for that module.
	V4D_MODULE_FUNC(void, StepSimulation, double deltaTime)
	
	V4D_MODULE_FUNC(bool, RayCastClosest, v4d::scene::Scene::RayCastHit* hit, glm::dvec3 origin, glm::dvec3 target, uint32_t mask)
	V4D_MODULE_FUNC(int, RayCastAll, std::vector<v4d::scene::Scene::RayCastHit>* hits, glm::dvec3 origin, glm::dvec3 target, uint32_t mask)
};
