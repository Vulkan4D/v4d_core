#include <v4d.h>

namespace v4d::scene {

	// Constructors and Destructor
	NetworkGameObject::NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent, Id id)
		: moduleID(moduleID), type(type), parent(parent), id(id) {}
	NetworkGameObject::NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent)
		: moduleID(moduleID), type(type), parent(parent), id(GetNextID()) {}
	// ~NetworkGameObject() {}
	
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
	
	NetworkGameObjectTransform NetworkGameObject::GetNetworkTransform() const {
		NetworkGameObjectTransform obj;
		std::lock_guard lock(mu);
		obj.SetFromTransformAndVelocity(transform, velocity);
		return obj;
	}
	
	void NetworkGameObject::SetTransformFromNetwork(const NetworkGameObjectTransform& obj) {
		std::lock_guard lock(mu);
		obj.GetTransformAndVelocity(transform, velocity);
	}
	
	void NetworkGameObject::SetTransform(const glm::dvec3 position, double angle, const glm::dvec3 axis) {
		std::lock_guard lock(mu);
		transform = glm::rotate(glm::translate(glm::dmat4(1), position), glm::radians(angle), axis);
	}
	
	void NetworkGameObject::SetVelocity(const glm::dvec3& v) {
		std::lock_guard lock(mu);
		velocity = v;
	}
	
	glm::dvec3 NetworkGameObject::GetLookDirection() const {
		std::lock_guard lock(mu);
		return glm::transpose(glm::dmat3(transform)) * glm::dvec3(0,0,1)/*forward*/;
	}
	
	glm::dvec3 NetworkGameObject::GetWorldPosition() const {
		std::lock_guard lock(mu);
		return transform[3];
	}
	
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

	void NetworkGameObject::UpdateObjectInstance() {
		if (objectInstance) {
			objectInstance->Lock();
			if (objectInstance->rigidbodyType == ObjectInstance::RigidBodyType::DYNAMIC || objectInstance->rigidbodyType == ObjectInstance::RigidBodyType::KINEMATIC) {
				if (isDynamic) {
					objectInstance->rigidbodyType = physicsControl? ObjectInstance::RigidBodyType::DYNAMIC : ObjectInstance::RigidBodyType::KINEMATIC;
				} else {
					objectInstance->rigidbodyType = ObjectInstance::RigidBodyType::STATIC;
				}
			}
			objectInstance->Unlock();
		}
	}
	
	void NetworkGameObject::UpdateObjectInstanceTransform() {
		if (objectInstance) {
			if (!posInit || !physicsControl) {
				objectInstance->Lock();
					if (!posInit && physicsControl) {
						objectInstance->AddImpulse(velocity * objectInstance->mass);
					}
					posInit = true;
					{std::lock_guard lock(mu);
						objectInstance->SetWorldTransform(transform); //TODO handle parent objects
					}
				objectInstance->Unlock();
			}
		}
	}
	
	void NetworkGameObject::ReverseUpdateObjectInstanceTransform() {
		objectInstance->Lock();
			{std::lock_guard lock(mu);
				transform = objectInstance->GetWorldTransform();
			}
		objectInstance->Unlock();
	}
	
	void NetworkGameObject::RemoveObjectInstance(Scene* scene) {
		if (objectInstance) {
			scene->RemoveObjectInstance(objectInstance);
			objectInstance = nullptr;
		}
	}
	
}

namespace v4d::networking::ZAP::data {
	void NetworkGameObjectTransform::SetFromTransformAndVelocity(const glm::dmat4& m, const glm::dvec3& v) {
		mat3x4.x0 = m[0][0];
		mat3x4.x1 = m[0][1];
		mat3x4.x2 = m[0][2];
		
		mat3x4.y0 = m[1][0];
		mat3x4.y1 = m[1][1];
		mat3x4.y2 = m[1][2];
		
		mat3x4.z0 = m[2][0];
		mat3x4.z1 = m[2][1];
		mat3x4.z2 = m[2][2];
		
		mat3x4.w0 = m[3][0];
		mat3x4.w1 = m[3][1];
		mat3x4.w2 = m[3][2];
		
		velocity.x = v.x;
		velocity.y = v.y;
		velocity.z = v.z;
	}
	void NetworkGameObjectTransform::GetTransformAndVelocity(glm::dmat4& m, glm::dvec3& v) const {
		m[0][0] = mat3x4.x0;
		m[0][1] = mat3x4.x1;
		m[0][2] = mat3x4.x2;
		m[0][3] = 0;
		
		m[1][0] = mat3x4.y0;
		m[1][1] = mat3x4.y1;
		m[1][2] = mat3x4.y2;
		m[1][3] = 0;
		
		m[2][0] = mat3x4.z0;
		m[2][1] = mat3x4.z1;
		m[2][2] = mat3x4.z2;
		m[2][3] = 0;
		
		m[3][0] = mat3x4.w0;
		m[3][1] = mat3x4.w1;
		m[3][2] = mat3x4.w2;
		m[3][3] = 1;
		
		v.x = velocity.x;
		v.y = velocity.y;
		v.z = velocity.z;
	}
}
