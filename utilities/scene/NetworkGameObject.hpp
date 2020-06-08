#pragma once

#include <v4d.h>

namespace v4d::networking::ZAP::data {
	
	ZAPDATA(DVector3,
		double x;
		double y;
		double z;
	)
	ZAPDATA(DMat3x4,
		double x0;
		double x1;
		double x2;
		double y0;
		double y1;
		double y2;
		double z0;
		double z1;
		double z2;
		double w0;
		double w1;
		double w2;
	)
	
	struct NetworkGameObjectTransform {
		ZAPABLE(NetworkGameObjectTransform) // 120 bytes
		
		DMat3x4 m;
		DVector3 velocity {0,0,0};
		
		void SetFromTransformAndVelocity(const glm::dmat4& mat, const glm::dvec3& v) {
			m.x0 = mat[0][0];
			m.x1 = mat[0][1];
			m.x2 = mat[0][2];
			
			m.y0 = mat[1][0];
			m.y1 = mat[1][1];
			m.y2 = mat[1][2];
			
			m.z0 = mat[2][0];
			m.z1 = mat[2][1];
			m.z2 = mat[2][2];
			
			m.w0 = mat[3][0];
			m.w1 = mat[3][1];
			m.w2 = mat[3][2];
			
			velocity.x = v.x;
			velocity.y = v.y;
			velocity.z = v.z;
		}
		
		void GetTransformAndVelocity(glm::dmat4& mat, glm::dvec3& v) const {
			mat[0][0] = m.x0;
			mat[0][1] = m.x1;
			mat[0][2] = m.x2;
			mat[0][3] = 0;
			
			mat[1][0] = m.y0;
			mat[1][1] = m.y1;
			mat[1][2] = m.y2;
			mat[1][3] = 0;
			
			mat[2][0] = m.z0;
			mat[2][1] = m.z1;
			mat[2][2] = m.z2;
			mat[2][3] = 0;
			
			mat[3][0] = m.w0;
			mat[3][1] = m.w1;
			mat[3][2] = m.w2;
			mat[3][3] = 1;
			
			v.x = velocity.x;
			v.y = velocity.y;
			v.z = velocity.z;
		}
	};
}

namespace v4d::scene {
	using namespace zapdata;
	struct NetworkGameObject {
		typedef uint32_t Type;
		typedef uint32_t Parent;
		typedef uint32_t Id;
		typedef uint32_t Attributes;
		typedef uint32_t Iteration;
		
		v4d::modular::ModuleID moduleID; // refers to a module with a submodule class of V4D_Objects
		Type type;
		Parent parent;
		Id id;
		
		// Attributes
		bool active = false;
		bool isDynamic = false;
		const std::vector<bool*> attributesPtrs {
			/* maximum of 32 variables */
			&active,
			&isDynamic,
		};
		
		ObjectInstancePtr objectInstance = nullptr;
		Iteration iteration = 1;
		glm::dmat4 transform {1};
		glm::dvec3 velocity {0};
		
		// Client-Side only
		bool posInit = false;
		bool physicsControl = false;
		
		// Server-Side only
		uint64_t physicsClientID = 0; // client id that controls physics for this object, 0 = server controls physics
		std::unordered_map<uint64_t/*clientID*/, Iteration /*iteration*/> clientIterations {};
		Id GetNextID() const {
			static std::atomic<Id> nextID = 1;
			return nextID++;
		}
		
		NetworkGameObjectTransform GetNetworkTransform() const {
			NetworkGameObjectTransform obj;
			obj.SetFromTransformAndVelocity(transform, velocity);
			return obj;
		}
		
		void SetTransformFromNetwork(const NetworkGameObjectTransform& obj) {
			obj.GetTransformAndVelocity(transform, velocity);
		}
		
		void SetTransform(const glm::dvec3 position = {0,0,0}, double angle = 0, const glm::dvec3 axis = {0,0,1}) {
			transform = glm::rotate(glm::translate(glm::dmat4(1), position), glm::radians(angle), axis);
		}
		
		glm::dvec3 GetLookDirection() const {
			return glm::transpose(glm::dmat3(transform)) * glm::dvec3(0,0,1)/*forward*/;
		}
		
		glm::dvec3 GetWorldPosition() const {
			return transform[3];
		}
		
		void SetAttributes(Attributes attrs) {
			for (size_t i = 0; i < attributesPtrs.size(); ++i) {
				*attributesPtrs[i] = attrs & (1 << i);
			}
		}
		Attributes GetAttributes() const {
			Attributes attrs = 0;
			for (size_t i = 0; i < attributesPtrs.size(); ++i) {
				attrs |= (*attributesPtrs[i]? 1:0) << i;
			}
			return attrs;
		}
		
		// Constructors and Destructor
		NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent, Id id)
		 : moduleID(moduleID), type(type), parent(parent), id(id) {}
		NetworkGameObject(v4d::modular::ModuleID moduleID, Type type, Parent parent = 0)
		 : moduleID(moduleID), type(type), parent(parent), id(GetNextID()) {}
		// ~NetworkGameObject() {}
		
		void UpdateObjectInstance() {
			if (objectInstance) {
				// objectInstance->Lock();
				if (objectInstance->rigidbodyType == ObjectInstance::RigidBodyType::DYNAMIC || objectInstance->rigidbodyType == ObjectInstance::RigidBodyType::KINEMATIC) {
					if (isDynamic) {
						objectInstance->rigidbodyType = physicsControl? ObjectInstance::RigidBodyType::DYNAMIC : ObjectInstance::RigidBodyType::KINEMATIC;
					} else {
						objectInstance->rigidbodyType = ObjectInstance::RigidBodyType::STATIC;
					}
				}
				// objectInstance->Unlock();
			}
		}
		
		void UpdateObjectInstanceTransform() {
			if (objectInstance && (!posInit || !physicsControl)) {
				if (!posInit && physicsControl) {
					objectInstance->AddImpulse(velocity * objectInstance->mass);
				}
				posInit = true;
				// objectInstance->Lock();
				objectInstance->SetWorldTransform(transform); //TODO handle parent objects
				// objectInstance->Unlock();
			}
		}
		
		void ReverseUpdateObjectInstanceTransform() {
			transform = objectInstance->GetWorldTransform();
		}
		
		void RemoveObjectInstance(Scene* scene) {
			if (objectInstance) {
				scene->RemoveObjectInstance(objectInstance);
				objectInstance = nullptr;
			}
		}
		
	};
	
	typedef std::shared_ptr<NetworkGameObject> NetworkGameObjectPtr;
}
