#include "PhysicsInfo.h"

namespace v4d::scene {

	std::atomic<uint32_t> PhysicsInfo::nextUniqueId = 0;
	
	PhysicsInfo::PhysicsInfo(RigidBodyType rigidbodyType, float mass) : uniqueId(nextUniqueId++), rigidbodyType(rigidbodyType), mass(mass) {}
	
	void PhysicsInfo::SetForce(glm::dvec3 forceDir, glm::dvec3 atPoint) {
		addedForce = (forceDir.length() > 0);
		forcePoint = atPoint;
		forceDirection = forceDir;
	}
	
	void PhysicsInfo::AddImpulse(glm::dvec3 impulseDir, glm::dvec3 atPoint) {
		if (impulseDir.length() == 0) return;
		physicsForceImpulses.emplace(impulseDir, atPoint);
	}
	
	void PhysicsInfo::AddLocalTorque(const glm::dvec3& torque) {
		appliedTorque += torque;
	}
	
	// Set Colliders
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index16* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(vertexCount);
		colliderMeshIndices16.resize(indexCount);
		memcpy(colliderMeshVertices.data(), vertices, vertexCount*sizeof(v4d::graphics::Mesh::VertexPosition));
		memcpy(colliderMeshIndices16.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index16));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::Index16* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(0);
		colliderMeshIndices16.resize(indexCount);
		memcpy(colliderMeshIndices16.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index16));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::VertexPosition* vertices, uint32_t vertexCount, v4d::graphics::Mesh::Index32* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(vertexCount);
		colliderMeshIndices32.resize(indexCount);
		memcpy(colliderMeshVertices.data(), vertices, vertexCount*sizeof(v4d::graphics::Mesh::VertexPosition));
		memcpy(colliderMeshIndices32.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index32));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetMeshCollider(v4d::graphics::Mesh::Index32* indices, uint32_t indexCount) {
		colliderMeshVertices.resize(0);
		colliderMeshIndices32.resize(indexCount);
		memcpy(colliderMeshIndices32.data(), indices, indexCount*sizeof(v4d::graphics::Mesh::Index32));
		colliderType = PhysicsInfo::ColliderType::MESH;
		colliderDirty = true;
		physicsDirty = true;
	}
	void PhysicsInfo::SetMeshCollider() {
		colliderMeshVertices.resize(0);
		colliderMeshIndices16.resize(0);
		colliderMeshIndices32.resize(0);
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
		boundingDistance = glm::max(size.x, glm::max(size.y, size.z))*1.42f;
		colliderType = PhysicsInfo::ColliderType::BOX;
		colliderDirty = true;
		physicsDirty = true;
	}
}
