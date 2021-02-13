#include <v4d.h>

namespace v4d::scene {
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
		bool colliderDirty = false;
		bool physicsDirty = false;
		float angularDamping = 0.001f;
		
		// Dynamic stuff
		float friction = 0.5f;
		float bounciness = 0.1f;
		glm::vec3 angularFactor = {1.0f,1.0f,1.0f};
		
		// Forces
		bool addedForce = false;
		glm::dvec3 forcePoint {0,0,0};
		glm::dvec3 forceDirection {0,0,0};
		glm::dvec3 appliedTorque {0,0,0};
		void SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint = {0,0,0});
		std::queue<std::tuple<glm::dvec3, glm::dvec3>> physicsForceImpulses {};
		void AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint = {0,0,0});
		void AddTorque(glm::dvec3);
		
		// Joints
		int32_t jointParent = -1; // must be the uniqueId of the parent's physics component
		glm::dmat4 localJointPoint {1};
		glm::dmat4 parentJointPoint {1};
		struct {float min, max;} jointTranslationLimitsX {0,0};
		struct {float min, max;} jointTranslationLimitsY {0,0};
		struct {float min, max;} jointTranslationLimitsZ {0,0};
		struct {float min, max;} jointRotationLimitsX {0,0};
		struct {float min, max;} jointRotationLimitsY {0,0};
		struct {float min, max;} jointRotationLimitsZ {0,0};
		glm::vec3 jointTranslationTarget {0};
		glm::vec3 jointRotationTarget {0};
		glm::vec3 jointTranslationMaxForce {0};
		glm::vec3 jointRotationMaxForce {0};
		glm::vec3 jointTranslationVelocity {0};
		glm::vec3 jointRotationVelocity {0};
		bool jointMotor = false;
		bool jointIsDirty = true;
		
		// Collision contacts / events
		int contacts = 0;
		
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
