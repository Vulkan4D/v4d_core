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
	
	struct V4DLIB NetworkGameObjectTransform {
		ZAPABLE(NetworkGameObjectTransform) // 120 bytes
		
		DMat3x4 mat3x4 {};
		DVector3 velocity {0,0,0};
		
		void SetFromTransformAndVelocity(const glm::dmat4&, const glm::dvec3&);
		void GetTransformAndVelocity(glm::dmat4&, glm::dvec3&) const;
	};
}

namespace v4d::scene {
	using namespace zapdata;
	class V4DLIB NetworkGameObject {
	public:
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
		
	private: 
		std::atomic<Iteration> iteration = 1;
		glm::dmat4 transform {1};
		glm::dvec3 velocity {0};
		mutable std::mutex mu;
		
	public:
		
		// Client-Side only
		bool posInit = false;
		bool physicsControl = false;
		
		// Server-Side only
		uint64_t physicsClientID = 0; // client id that controls physics for this object, 0 = server controls physics
		std::unordered_map<uint64_t/*clientID*/, Iteration /*iteration*/> clientIterations {};
		Id GetNextID() const;
		Iteration Iterate();
		
		Iteration GetIteration() const;
		Iteration SetIteration(Iteration i);
		
		NetworkGameObjectTransform GetNetworkTransform() const;
		void SetTransformFromNetwork(const NetworkGameObjectTransform&);
		
		void SetTransform(const glm::dvec3 position = {0,0,0}, double angle = 0, const glm::dvec3 axis = {0,0,1});
		void SetVelocity(const glm::dvec3&);
		
		glm::dvec3 GetLookDirection() const;
		glm::dvec3 GetWorldPosition() const;
		
		void SetAttributes(Attributes attrs);
		Attributes GetAttributes() const;
		
		// Constructors and Destructor
		NetworkGameObject(v4d::modular::ModuleID, Type, Parent, Id);
		NetworkGameObject(v4d::modular::ModuleID, Type, Parent = 0);
		
		void UpdateObjectInstance();
		void UpdateObjectInstanceTransform();
		void ReverseUpdateObjectInstanceTransform();
		void RemoveObjectInstance(Scene*);
	};
	
	typedef std::shared_ptr<NetworkGameObject> NetworkGameObjectPtr;
	typedef std::unordered_map<NetworkGameObject::Id, NetworkGameObjectPtr> NetworkGameObjects;
}
