#pragma once

#include <v4d.h>

namespace v4d::networking::ZAP::data {
	
	ZAPDATA(DVector3,
		double x;
		double y;
		double z;
	)
	ZAPDATA(DQuaternion,
		double x;
		double y;
		double z;
		double w;
	)
	
	struct NetworkGameObjectTransform {
		ZAPABLE(NetworkGameObjectTransform) // 80 bytes
		
		DVector3 position {0,0,0};
		DQuaternion rotation {0,0,0,1};
		DVector3 velocity {0,0,0};
		
		void SetFromTransformAndVelocity(const glm::dmat4& mat, const glm::dvec3& v) {
			glm::dvec3 pos = glm::dvec3(mat[3]);
			glm::dquat rot = glm::quat_cast(glm::transpose(glm::dmat3(mat)));
			position.x = pos.x;
			position.y = pos.y;
			position.z = pos.z;
			rotation.x = rot.x;
			rotation.y = rot.y;
			rotation.z = rot.z;
			rotation.w = rot.w;
			velocity.x = v.x;
			velocity.y = v.y;
			velocity.z = v.z;
		}
		
		void GetTransformAndVelocity(glm::dmat4& mat, glm::dvec3& v) const {
			mat = glm::mat4_cast(glm::dquat(rotation.x, rotation.y, rotation.z, rotation.w)) * glm::translate(glm::dmat4{1}, glm::dvec3(position.x, position.y, position.z));
			v.x = velocity.x;
			v.y = velocity.y;
			v.z = velocity.z;
		}
		
		// void Read(v4d::data::Stream* stream) {
		// 	*stream
		// 		>> position
		// 		>> rotation
		// 		>> velocity
		// 	;
		// }
		// void Write(v4d::data::Stream* stream) const {
		// 	*stream
		// 		<< position
		// 		<< rotation
		// 		<< velocity
		// 	;
		// }
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
		// bool hasRigidbodies = false;
		// bool isDynamic = false;
		
		ObjectInstancePtr objectInstance = nullptr;
		Iteration iteration = 0;
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
			return glm::transpose(glm::dmat3(transform)) * glm::dvec3(0,1,0)/*forward*/;
		}
		
		glm::dvec3 GetWorldPosition() const {
			return transform[3];
		}
		
		const std::array<bool*, 1/* max 32 */> attributesPtrs {
			&active,
		};
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
				
				objectInstance->rigidbodyType = physicsControl? ObjectInstance::RigidBodyType::DYNAMIC : ObjectInstance::RigidBodyType::KINEMATIC;
				objectInstance->physicsDirty = true;
				
				// objectInstance->Unlock();
			}
		}
		
		void UpdateObjectInstanceTransform() {
			if (objectInstance && (!posInit || !physicsControl)) {
				posInit = true;
				// objectInstance->Lock();
				objectInstance->SetWorldTransform(transform); //TODO handle parent objects
				// objectInstance->Unlock();
			}
		}
		
		void ReverseUpdateObjectInstanceTransform() {
			transform = objectInstance->GetWorldTransform();
		}
		
	};
	
	typedef std::shared_ptr<NetworkGameObject> NetworkGameObjectPtr;
}
