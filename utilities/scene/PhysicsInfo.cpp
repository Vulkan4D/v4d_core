#include <v4d.h>

namespace v4d::scene {

	std::atomic<uint32_t> PhysicsInfo::nextUniqueId = 0;
	
	PhysicsInfo::PhysicsInfo(RigidBodyType rigidbodyType, double mass) : uniqueId(nextUniqueId++), rigidbodyType(rigidbodyType), mass(mass) {}
	
	void PhysicsInfo::SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint) {
		addedForce = (forceDir.length() > 0);
		forcePoint = atPoint;
		forceDirection = forceDir;
	}
	
	void PhysicsInfo::AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint) {
		if (impulseDir.length() == 0) return;
		physicsForceImpulses.emplace(impulseDir, atPoint);
	}
	
	// Set Colliders
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(vertexCount);
		colliderMeshIndices.resize(indexCount);
		memcpy(colliderMeshVertices.data(), vertices, vertexCount*sizeof(v4d::graphics::Mesh::VertexPosition));
		memcpy(colliderMeshIndices.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::Index* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(0);
		colliderMeshIndices.resize(indexCount);
		memcpy(colliderMeshIndices.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetSphereCollider(float radius) {
		boundingDistance = radius;
		colliderType = PhysicsInfo::ColliderType::SPHERE;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetBoxCollider(glm::vec3 size) {
		boundingBoxSize = size;
		colliderType = PhysicsInfo::ColliderType::BOX;
		colliderDirty = true;
		physicsDirty = true;
	}
}
