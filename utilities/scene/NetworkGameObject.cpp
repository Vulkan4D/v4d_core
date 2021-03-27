#include "NetworkGameObject.h"

namespace v4d::scene {

	// Constructors
	NetworkGameObject::NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent, Id id)
		: moduleID(moduleID), type(type), parent(parent), id(id) {}
	NetworkGameObject::NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent)
		: moduleID(moduleID), type(type), parent(parent), id(GetNextID()) {}
	
	NetworkGameObject::Id NetworkGameObject::GetNextID() const {
		static std::atomic<Id> nextID = 1;
		return nextID++;
	}
	NetworkGameObject::Iteration NetworkGameObject::Iterate() {
		return (Iteration)++iteration;
	}
	
	NetworkGameObject::Iteration NetworkGameObject::GetIteration() const {
		return (Iteration)iteration;
	}
	NetworkGameObject::Iteration NetworkGameObject::SetIteration(Iteration i) {
		return iteration = i;
	}
	
	// glm::dvec3 NetworkGameObject::GetLookDirection() const {
	// 	std::lock_guard lock(mu);
	// 	return glm::transpose(glm::dmat3(transform)) * glm::dvec3(0,0,1)/*forward*/;
	// }
	
	void NetworkGameObject::SetAttributes(Attributes attrs) {
		for (size_t i = 0; i < attributesPtrs.size(); ++i) {
			*attributesPtrs[i] = attrs & (1 << i);
		}
	}
	NetworkGameObject::Attributes NetworkGameObject::GetAttributes() const {
		Attributes attrs = 0;
		for (size_t i = 0; i < attributesPtrs.size(); ++i) {
			attrs |= (*attributesPtrs[i]? 1:0) << i;
		}
		return attrs;
	}

	void NetworkGameObject::UpdateGameObject() {
		std::lock_guard lock(mu);
		if (auto entity = renderableGeometryEntityInstance.lock(); entity) {
			entity->parentId = parent;
			entity->extra = extra;
			if (auto physics = entity->physics.Lock(); physics) {
				// if (physics->rigidbodyType == PhysicsInfo::RigidBodyType::DYNAMIC || physics->rigidbodyType == PhysicsInfo::RigidBodyType::KINEMATIC) {
					if (isDynamic) {
						physics->rigidbodyType = physicsControl? PhysicsInfo::RigidBodyType::DYNAMIC : PhysicsInfo::RigidBodyType::KINEMATIC;
					} else {
						physics->rigidbodyType = PhysicsInfo::RigidBodyType::STATIC;
					}
				// }
			}
		}
	}
	
	void NetworkGameObject::UpdateGameObjectTransform() {
		std::lock_guard lock(mu);
		if (!posInit) {
			position = targetPosition;
			orientation = targetOrientation;
			if (auto entity = renderableGeometryEntityInstance.lock(); entity) {
				entity->SetLocalTransform(glm::translate(glm::dmat4(1), position) * glm::mat4_cast(orientation));
				// if (velocity != glm::dvec3(0)) {
				// 	if (auto physics = entity->physics.Lock(); physics) {
				// 		physics->AddImpulse(velocity * double(physics->mass));
				// 	}
				// }
			}
			posInit = true;
		}
	}
	
	void NetworkGameObject::SmoothlyInterpolateGameObjectTransform(double delta) {
		if (auto entity = renderableGeometryEntityInstance.lock(); entity) {
			if (delta > 0) {
				position = glm::mix(position, targetPosition, delta);
				orientation = glm::slerp(orientation, targetOrientation, delta);
			} else {
				position = targetPosition;
				orientation = targetOrientation;
			}
			entity->SetLocalTransform(glm::translate(glm::dmat4(1), position) * glm::mat4_cast(orientation));
		}
	}
	
	bool NetworkGameObject::ReverseUpdateGameObjectTransform() {
		std::lock_guard lock(mu);
		if (auto entity = renderableGeometryEntityInstance.lock(); entity) {
			const auto& t = entity->GetLocalTransform();
			targetPosition = t[3];
			targetOrientation = glm::quat_cast(t);
			if (position != targetPosition || orientation != targetOrientation) {
				position = targetPosition;
				orientation = targetOrientation;
				return true;
			}
		}
		return false;
	}
	
	void NetworkGameObject::RemoveGameObject() {
		std::lock_guard lock(mu);
		if (auto entity = renderableGeometryEntityInstance.lock(); entity) {
			entity->Destroy();
		}
	}
	
}
