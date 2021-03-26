#pragma once

#include <v4d.h>
#include <vector>
#include <atomic>
#include <tuple>
#include <queue>
#include <optional>
#include "utilities/graphics/vulkan/Loader.h"
#include "utilities/graphics/Mesh.hpp"

namespace v4d::scene {
	
	struct Collision {
		uint32_t collisionInstance; // objectInstanceA | collision options
		uint32_t objectInstanceB; // objectInstanceB | collision Ray Mask
		
		int32_t objectGeometryA;
		int32_t objectGeometryB;
		
		glm::vec4 position; // view-space, w = depth (how much under the object's surface this start position is)
		glm::vec4 direction; // view-space direction, w = max travel distance from surface (typically velocity*deltaTime)
		
		glm::vec4 contactA;
		glm::vec4 contactB;
		
		glm::vec4 normalA;
		glm::vec4 normalB;
		
		Collision(uint32_t objectInstanceId, uint8_t collisionIndex, uint8_t rayMask, glm::vec4 startPositionViewSpaceAndBoundingDistance, glm::vec4 directionViewSpaceAndMaxTravelDistance)
		: collisionInstance( (objectInstanceId << 8) | collisionIndex )
		, objectInstanceB( rayMask )
		, objectGeometryA(-1)
		, objectGeometryB(-1)
		, position(startPositionViewSpaceAndBoundingDistance)
		, direction(directionViewSpaceAndMaxTravelDistance)
		, contactA(0)
		, contactB(0)
		{}
	};

	struct V4DLIB PhysicsInfo {
		enum class ColliderType : int {
			NONE = 0,
			MESH,
			SPHERE,
			BOX,
			STATIC_PLANE,
			HEIGHTFIELD,
		};
		enum class RigidBodyType : int {
			NONE = 0,
			KINEMATIC,
			STATIC,
			DYNAMIC
		};
		
		uint32_t uniqueId;
		RigidBodyType rigidbodyType;
		float mass;
		
		static std::atomic<uint32_t> nextUniqueId;
		
		PhysicsInfo(RigidBodyType rigidbodyType = RigidBodyType::NONE, float mass = 0);
		
		// Collider
		ColliderType colliderType = ColliderType::NONE;
		std::vector<v4d::graphics::Mesh::VertexPosition> colliderMeshVertices {};
		std::vector<v4d::graphics::Mesh::Index16> colliderMeshIndices16 {};
		std::vector<v4d::graphics::Mesh::Index32> colliderMeshIndices32 {};
		float boundingDistance = 0;
		glm::vec3 boundingBoxSize {0};
		
		// RigidBody
		glm::dvec3 centerOfMass {0,0,0};
		float friction = 0.5f;
		float bounciness = 0.1f;
		// glm::vec3 angularFactor = {1.0f,1.0f,1.0f};
		// float angularDamping = 0.001f;
		bool colliderDirty = false;
		bool physicsDirty = false;
		
		// // Updated by the physics module
		// v4d::Timer timer {false};
		// glm::dvec3 gForce {0,0,0};
		glm::dvec3 linearVelocity {0,0,0};
		glm::dvec3 angularVelocity {0,0,0};
		
		
		// Forces
		bool addedForce = false;
		glm::dvec3 forcePoint {0,0,0};
		glm::dvec3 forceDirection {0,0,0};
		glm::dvec3 appliedTorque {0,0,0};
		void SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint = {0,0,0});
		std::queue<std::tuple<glm::dvec3, glm::dvec3>> physicsForceImpulses {};
		void AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint = {0,0,0});
		void AddLocalTorque(const glm::dvec3&);
		
		// // Joints
		// int32_t jointParent = -1; // must be the uniqueId of the parent's physics component
		// glm::dmat4 localJointPoint {1};
		// glm::dmat4 parentJointPoint {1};
		// struct {float min, max;} jointTranslationLimitsX {0,0};
		// struct {float min, max;} jointTranslationLimitsY {0,0};
		// struct {float min, max;} jointTranslationLimitsZ {0,0};
		// struct {float min, max;} jointRotationLimitsX {0,0};
		// struct {float min, max;} jointRotationLimitsY {0,0};
		// struct {float min, max;} jointRotationLimitsZ {0,0};
		// glm::vec3 jointTranslationTarget {0};
		// glm::vec3 jointRotationTarget {0};
		// glm::vec3 jointTranslationMaxForce {0};
		// glm::vec3 jointRotationMaxForce {0};
		// glm::vec3 jointTranslationVelocity {0};
		// glm::vec3 jointRotationVelocity {0};
		// bool jointMotor = false;
		// bool jointIsDirty = true;
		
		// int contacts = 0;
		
		struct CollisionTest {
			std::vector<Collision> collisions;
			std::optional<glm::dmat4> worldTransformBefore;
			std::optional<glm::dmat4> worldTransformAfter;
			CollisionTest() : collisions(), worldTransformBefore(std::nullopt), worldTransformAfter(std::nullopt) {};
			CollisionTest(const glm::dmat4& worldTransform, const glm::dvec3& velocity, double deltaTime) 
			: collisions()
			, worldTransformBefore(worldTransform)
			, worldTransformAfter(glm::translate(glm::dmat4(1), velocity * deltaTime) * worldTransform)
			{}
		} collisionTest {};
		
		// Set Colliders
		void SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index16* indices, uint32_t indexCount);
		void SetMeshCollider(v4d::graphics::Mesh::Index16* indices, uint32_t indexCount);
		void SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index32* indices, uint32_t indexCount);
		void SetMeshCollider(v4d::graphics::Mesh::Index32* indices, uint32_t indexCount);
		void SetMeshCollider();
		void SetSphereCollider(float radius);
		void SetBoxCollider(glm::vec3 size);
	};
}
