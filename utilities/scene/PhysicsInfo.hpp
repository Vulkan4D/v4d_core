#include <v4d.h>

namespace v4d::scene {
	struct PhysicsInfo {
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
		
		RigidBodyType rigidbodyType;
		double mass;
		
		PhysicsInfo(RigidBodyType rigidbodyType = RigidBodyType::NONE, double mass = 0) : rigidbodyType(rigidbodyType), mass(mass) {}
		
		// Collider
		ColliderType colliderType = ColliderType::NONE;
		std::vector<v4d::graphics::Mesh::VertexPosition> colliderMeshVertices {};
		std::vector<v4d::graphics::Mesh::Index> colliderMeshIndices {};
		float boundingDistance = 0;
		glm::vec3 boundingBoxSize {0};
		
		// RigidBody
		glm::dvec3 centerOfMass {0,0,0};
		void* physicsObject = nullptr;
		void* colliderShapeObject = nullptr;
		bool colliderDirty = true;
		bool physicsDirty = false;
		bool physicsActive = true;
		
		// Forces
		bool addedForce = false;
		glm::dvec3 forcePoint {0,0,0};
		glm::dvec3 forceDirection {0,0,0};
		void SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint = {0,0,0}) {
			addedForce = (forceDir.length() > 0);
			forcePoint = atPoint;
			forceDirection = forceDir;
		}
		std::queue<std::tuple<glm::dvec3, glm::dvec3>> physicsForceImpulses {};
		void AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint = {0,0,0}) {
			if (impulseDir.length() == 0) return;
			physicsForceImpulses.emplace(impulseDir, atPoint);
		}
		
		// Set Colliders
		void SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index* indices, uint32_t indexCount) {
			colliderMeshVertices.resize(vertexCount);
			colliderMeshIndices.resize(indexCount);
			memcpy(colliderMeshVertices.data(), vertices, vertexCount*sizeof(v4d::graphics::Mesh::VertexPosition));
			memcpy(colliderMeshIndices.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index));
			colliderType = PhysicsInfo::ColliderType::MESH;
			colliderDirty = true;
			physicsDirty = true;
		}
		void SetMeshCollider(v4d::graphics::Mesh::Index* indices, uint32_t indexCount) {
			colliderMeshVertices.resize(0);
			colliderMeshIndices.resize(indexCount);
			memcpy(colliderMeshIndices.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index));
			colliderType = PhysicsInfo::ColliderType::MESH;
			colliderDirty = true;
			physicsDirty = true;
		}
		void SetSphereCollider(float radius) {
			boundingDistance = radius;
			colliderType = PhysicsInfo::ColliderType::SPHERE;
			colliderDirty = true;
			physicsDirty = true;
		}
		void SetBoxCollider(glm::vec3 size) {
			boundingBoxSize = size;
			colliderType = PhysicsInfo::ColliderType::BOX;
			colliderDirty = true;
			physicsDirty = true;
		}
	};
}
