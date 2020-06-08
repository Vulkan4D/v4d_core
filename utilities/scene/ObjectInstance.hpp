#pragma once

#include <v4d.h>
#include "Geometry.h"

namespace v4d::scene {
	using namespace v4d::graphics;
	
	struct GeometryInstance {
		std::shared_ptr<Geometry> geometry;
		int rayTracingInstanceIndex;
		glm::dmat4 transform;
		std::string type;
		
		GeometryInstance(std::shared_ptr<Geometry> geometry, int rayTracingInstanceIndex = -1, glm::dmat4 transform = glm::dmat4 {1}, std::string type = "standard")
		: geometry(geometry), rayTracingInstanceIndex(rayTracingInstanceIndex), transform(transform), type(type) {};
	};
	
	class V4DLIB ObjectInstance {
		
	private:
		glm::dmat4 transform {1};
		std::vector<GeometryInstance> geometries {};
		std::vector<LightSource*> lightSources {};
		void (*generateFunc)(ObjectInstance*) = nullptr;
		
	private: // Cached data
	friend Geometry;
		std::optional<bool> isProcedural;
		uint32_t objectOffset = 0;
		bool geometriesDirty = true;
		bool active = true;
		bool generated = false;
		mutable std::mutex mu;
		
	public:
	
		#pragma region Physics
		
			// Reserved for use by V4D_Physics module class
			enum class RigidBodyType : int {
				NONE = 0,
				KINEMATIC,
				STATIC,
				DYNAMIC
			};
			RigidBodyType rigidbodyType = RigidBodyType::NONE;
			double mass = 0;
			glm::dvec3 centerOfMass {0,0,0};
			void* physicsObject = nullptr;
			bool physicsDirty = false;
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
		
		#pragma endregion
		
		#pragma region Ray Tracing
		
		uint32_t rayTracingMaskAdded = 0x0;
		uint32_t rayTracingMaskRemoved = 0x0;
		
		#pragma endregion
	
		#pragma region Constructor/Destructor
		
		ObjectInstance() {
			Geometry::globalBuffers.AddObject(this);
		}
		
		~ObjectInstance() {
			Disable();
			ClearGeometries();
			RemoveLightSources();
			Geometry::globalBuffers.RemoveObject(this);
		}
		
		#pragma endregion
		
		#pragma region Write to device
		
		void GenerateGeometries() {
			{
				if (generateFunc && !generated) generateFunc(this);
				SetGenerated();
			}
			WriteGeometriesInformation();
		}
		
		void WriteGeometriesInformation() {
			for (auto& geom : geometries) {
				if (!geom.geometry->geometryInfoInitialized) geom.geometry->SetGeometryInfo(objectOffset, geom.geometry->material);
			}
		}
		
		void PushGeometries(Device* device, VkCommandBuffer cmdBuffer) {
			GenerateGeometries();
			if (geometriesDirty) {
				for (auto& geom : geometries) {
					geom.geometry->AutoPush(device, cmdBuffer);
				}
			}
			geometriesDirty = false;
		}
		
		void WriteMatrices(const glm::dmat4& viewMatrix) {
			Geometry::globalBuffers.WriteObject(this);
			
			for (auto& geom : geometries) {
				geom.geometry->SetGeometryTransform(transform * glm::dmat4(geom.transform), viewMatrix);
			}
		
			for (auto* lightSource : lightSources) {
				lightSource->viewSpacePosition = viewMatrix * transform * glm::dvec4(lightSource->position, 1);
				Geometry::globalBuffers.WriteLight(lightSource);
			}
		}
		
		#pragma endregion
		
		#pragma region Configure
		
		void SetTranslation(const glm::dvec3 t) {
			transform = glm::translate(transform, t);
		}
		
		void SetRotation(double angle, const glm::dvec3& axis = {0,0,1}) {
			transform = glm::rotate(transform, glm::radians(angle), axis);
		}
		
		void SetGenerateFunc(void (*genFunc)(ObjectInstance*)) {
			generateFunc = genFunc;
		}
		
		void Configure(void (*genFunc)(ObjectInstance*), const glm::dvec3 position = {0,0,0}, double angle = 0, const glm::dvec3 axis = {0,0,1}) {
			transform = glm::rotate(glm::translate(glm::dmat4(1), position), glm::radians(angle), axis);
			generateFunc = genFunc;
		}
		
		#pragma endregion
		
		#pragma region Generate
		
		std::shared_ptr<Geometry> AddGeometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material = 0, glm::dmat4 transform = glm::dmat4{1}) {
			return AddGeometry("standard", vertexCount, indexCount, material, transform);
		}
		std::shared_ptr<Geometry> AddGeometry(const std::string& type, uint32_t vertexCount, uint32_t indexCount, uint32_t material = 0, glm::dmat4 transform = glm::dmat4{1}) {
			if (!isProcedural.has_value()) isProcedural = false;
			if (isProcedural.value()) {
				LOG_ERROR("A sphere/procedural object cannot contain triangle geometries")
				return nullptr;
			}
			return geometries.emplace_back(std::make_shared<Geometry>(vertexCount, indexCount, material), -1, transform, type).geometry;
		}
		
		std::shared_ptr<Geometry> AddProceduralGeometry(const std::string& type, uint32_t count, uint32_t material = 0, glm::dmat4 transform = glm::dmat4{1}) {
			if (!isProcedural.has_value()) isProcedural = true;
			if (!isProcedural.value()) {
				LOG_ERROR("A procedural object cannot contain triangle geometries")
				return nullptr;
			}
			auto geometry = geometries.emplace_back(std::make_shared<Geometry>(count, 0, material, true), -1, transform, type).geometry;
			geometry->colliderType = Geometry::ColliderType::BOX;
			return geometry;
		}
		
		std::shared_ptr<Geometry> AddGeometry(std::shared_ptr<Geometry> templaceGeometry, glm::dmat4 transform = glm::dmat4{1}) {
			return AddGeometry("standard", templaceGeometry, transform);
		}
		std::shared_ptr<Geometry> AddGeometry(const std::string& type, std::shared_ptr<Geometry> templaceGeometry, glm::dmat4 transform = glm::dmat4{1}) {
			return geometries.emplace_back(std::make_shared<Geometry>(templaceGeometry), -1, transform, type).geometry;
		}
		
		LightSource* AddLightSource(
			glm::dvec3 position,
			glm::f32 intensity,
			glm::vec3 color = {1,1,1},
			glm::u32 type = 1,
			glm::u32 attributes = 0,
			glm::f32 radius = 0
		) {
			LightSource* lightSource = lightSources.emplace_back(new LightSource(
				position,
				intensity,
				color,
				type,
				attributes,
				radius
			));
			Geometry::globalBuffers.AddLight(lightSource);
			return lightSource;
		}
		
		void SetSphereGeometry(const std::string& type, float radius, glm::vec4 color = {0,0,0,0}, uint32_t material = 0, float custom1 = 0) {
			if (!isProcedural.has_value()) isProcedural = true;
			if (!isProcedural.value()) {
				LOG_ERROR("A sphere cannot contain triangle geometries")
				return;
			}
			if (geometries.size() == 0) {
				geometries.emplace_back(std::make_shared<Geometry>(1, 0, material, true), -1, glm::dmat4{1}, type);
			}
			geometries[0].geometry->SetProceduralVertex(0, glm::vec3(-radius), glm::vec3(+radius), color, custom1);
			geometries[0].geometry->colliderType = Geometry::ColliderType::SPHERE;
			geometries[0].geometry->boundingDistance = radius;
		}
		
		LightSource* SetSphereLightSource(
			const std::string& type, 
			float radius,
			glm::f32 lightIntensity,
			glm::vec3 lightColor = {1,1,1},
			glm::u32 lightType = 1,
			glm::u32 lightAttributes = 0,
			float custom1 = 0,
			glm::vec4 geomColor = {0,0,0,0}
		) {
			lightSources.clear();
			auto* lightSource = AddLightSource({0,0,0}, lightIntensity, lightColor, lightType, lightAttributes, radius);
			if (radius > 0) SetSphereGeometry(type, radius, geomColor, lightSource->lightOffset, custom1);
			geometries[0].geometry->rayTracingMask = GEOMETRY_ATTR_PRIMARY_VISIBLE | GEOMETRY_ATTR_EMITTER | GEOMETRY_ATTR_COLLIDER | GEOMETRY_ATTR_REFLECTION_VISIBLE;
			return lightSource;
		}
		
		#pragma endregion
		
		#pragma region Clear
		
		void RemoveGeometries() {
			geometries.clear();
			generated = false;
		}
		
		void RemoveLightSources() {
			for (auto* lightSource : lightSources) {
				Geometry::globalBuffers.RemoveLight(lightSource);
				delete lightSource;
			}
			lightSources.clear();
		}
		
		void ClearGeometries() {
			geometriesDirty = true;
			RemoveGeometries();
			RemoveLightSources();
		}
		
		#pragma endregion
		
		#pragma region Getters/Setters
		
		glm::dmat4 GetWorldTransform() const {
			return transform;
		}
		
		int GetFirstGeometryOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0].geometry->geometryOffset;
		}
		
		int GetFirstGeometryVertexOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0].geometry->vertexOffset;
		}
		
		int GetFirstGeometryIndexOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0].geometry->indexOffset;
		}
		
		void GetGeometriesTotals(int* totalVertices, int* totalIndices) const {
			for (auto& geom : geometries) {
				*totalVertices += geom.geometry->vertexCount;
				*totalIndices += geom.geometry->indexCount;
			}
		}
		
		bool IsProcedural() const {return isProcedural.has_value() && isProcedural.value();}
		
		uint32_t GetObjectOffset() const {return objectOffset;}
		
		void SetGeometriesDirty(bool dirty = true) {geometriesDirty = dirty;}
		bool IsGeometriesDirty() const {return geometriesDirty;}
		
		int CountGeometries() const {return geometries.size();}
		
		std::vector<GeometryInstance>& GetGeometries() {return geometries;}
		std::vector<LightSource*>& GetLightSources() {return lightSources;}
		
		#pragma endregion
		
		#pragma region Realtime / Per-Frame Updates
		
		void SetWorldTransform(const glm::dmat4 worldSpaceTransform) {
			transform = worldSpaceTransform;
		}
		
		void SetVertex(uint32_t geomIndex, uint32_t vertIndex, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color) {
			geometries[geomIndex].geometry->SetVertex(vertIndex, pos, normal, uv, color);
			SetGeometriesDirty();
		}
		
		void SetProceduralVertex(uint32_t geomIndex, uint32_t vertIndex, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1 = 0) {
			geometries[geomIndex].geometry->SetProceduralVertex(vertIndex, aabbMin, aabbMax, color, custom1);
			SetGeometriesDirty();
		}
		
		void Lock() const {
			mu.lock();
		}
		
		void Unlock() const {
			mu.unlock();
		}
		
		#pragma endregion
		
		#pragma region Active/Inactive
		
		bool HasActiveGeometries() const {
			for (auto& g : geometries) {
				if (g.geometry->active) return true;
			}
			return false;
		}
		
		bool IsActive() const {
			return active;
		}
		
		void Enable() {
			if (!active && rigidbodyType != RigidBodyType::NONE) physicsDirty = true;
			active = true;
		}
		
		void Disable() {
			if (active && physicsObject) physicsDirty = true;
			active = false;
		}
		
		bool IsGenerated() const {
			return generated;
		}
		
		void SetGenerated() {
			if (rigidbodyType != RigidBodyType::NONE) physicsDirty = true;
			generated = true;
		}
		
		#pragma endregion
		
	};
	
}
