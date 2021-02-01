#include <v4d.h>

namespace v4d::scene {
	struct V4DLIB PhysicsInfo {
		enum class ColliderType : int {
			NONE = 0,
			MESH,
			SPHERE,
			BOX,
			STATIC_PLANE,
		};
		enum class RigidBodyType : int {
			NONE = 0,
			KINEMATIC,
			STATIC,
			DYNAMIC
		};
		
		uint32_t uniqueId;
		RigidBodyType rigidbodyType;
		double mass;
		
		static std::atomic<uint32_t> nextUniqueId;
		
		PhysicsInfo(RigidBodyType rigidbodyType = RigidBodyType::NONE, double mass = 0);
		
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
		
		// Forces
		bool addedForce = false;
		glm::dvec3 forcePoint {0,0,0};
		glm::dvec3 forceDirection {0,0,0};
		void SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint = {0,0,0});
		std::queue<std::tuple<glm::dvec3, glm::dvec3>> physicsForceImpulses {};
		void AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint = {0,0,0});
		
		// Joints
		int32_t p2pJointParent = -1; // must be the uniqueId of the parent's physics component
		glm::dvec3 localPivotPoint {0,0,0};
		glm::dvec3 pivotPointInParent {0,0,0};
		
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
