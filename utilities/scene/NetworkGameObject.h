#pragma once

#include <v4d.h>
#include <any>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/RenderableGeometryEntity.h"

namespace v4d::scene {
	class V4DLIB NetworkGameObject {
	public:
		using Type = uint32_t;
		using Parent = uint64_t;
		using Id = uint64_t;
		using Extra = uint64_t; 
		using Attributes = uint32_t;
		using Iteration = uint32_t;
		using Position = glm::dvec3;
		using Orientation = glm::dquat;
		
		v4d::modular::ModuleID moduleID; // refers to a module with a submodule class of V4D_Objects
		Type type;
		Parent parent;
		Id id;
		Extra extra;
		
		std::weak_ptr<v4d::graphics::RenderableGeometryEntity> renderableGeometryEntityInstance;
		std::any entityData;
		
		// Attributes
		bool active = false;
		bool isDynamic = false;
		const std::vector<bool*> attributesPtrs {
			/* maximum of 32 variables */
			&active,
			&isDynamic,
		};
		
	private: 
		std::atomic<Iteration> iteration = 1;
		mutable std::mutex mu;
		
	public:
		Position position {0};
		Orientation orientation {1,0,0,0};
		
		// Client-Side only
		bool posInit = false;
		bool physicsControl = false;
		Position targetPosition {0};
		Orientation targetOrientation {1,0,0,0};
		
		// Server-Side only
		uint64_t physicsClientID = 0; // client id that controls physics for this object, 0 = server controls physics
		std::unordered_map<uint64_t/*clientID*/, Iteration /*iteration*/> clientIterations {};
		Id GetNextID() const;
		Iteration Iterate();
		
		Iteration GetIteration() const;
		Iteration SetIteration(Iteration i);
		
		void SmoothlyInterpolateGameObjectTransform(double delta);
		
		void SetAttributes(Attributes attrs);
		Attributes GetAttributes() const;
		
		// Constructors
		NetworkGameObject(v4d::modular::ModuleID, Type, Parent, Id);
		NetworkGameObject(v4d::modular::ModuleID, Type, Parent = 0);
		
		void UpdateGameObject();
		void UpdateGameObjectTransform();
		bool ReverseUpdateGameObjectTransform();
		void RemoveGameObject();
	};
	
	typedef std::shared_ptr<NetworkGameObject> NetworkGameObjectPtr;
	typedef std::unordered_map<NetworkGameObject::Id, NetworkGameObjectPtr> NetworkGameObjects;
}
