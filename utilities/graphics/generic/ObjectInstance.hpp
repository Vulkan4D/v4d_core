#pragma once

#include <v4d.h>
#include "Geometry.h"

namespace v4d::graphics {

	class V4DLIB ObjectInstance {
		std::string objectType;
		
	private:
		glm::dmat4 transform {1};
		std::vector<Geometry*> geometries {};
		std::vector<LightSource*> lightSources {};
		glm::vec3 custom3 {0};
		glm::vec4 custom4 {0};
		void (*generateFunc)(ObjectInstance*) = nullptr;
		enum : uint32_t {
			RAY_TRACING_TYPE_SOLID = 0x01,
			RAY_TRACING_TYPE_LIQUID = 0x02,
			RAY_TRACING_TYPE_CLOUD = 0x04,
			RAY_TRACING_TYPE_PARTICLE = 0x08,
			RAY_TRACING_TYPE_TRANSPARENT = 0x10,
			RAY_TRACING_TYPE_CUTOUT = 0x20,
			RAY_TRACING_TYPE_CELESTIAL = 0x40,
			RAY_TRACING_TYPE_EMITTER = 0x80,
		};
		uint32_t rayTracingMask = RAY_TRACING_TYPE_SOLID;
		
	private: // Cached data
	friend Geometry;
		std::optional<bool> isProcedural;
		uint32_t objectOffset = 0;
		int rayTracingBlasIndex = -1;
		int rayTracingInstanceIndex = -1;
		glm::mat4 modelViewMatrix {1};
		glm::mat3 normalMatrix {1};
		glm::mat3x4 rayTracingModelViewMatrix {1};
		bool geometriesDirty = true;
		bool active = true;
		bool generated = false;
		bool markedForDeletion = false;
		
	public:
		#pragma region Constructor/Destructor
		
		ObjectInstance(std::string type = "standard") : objectType(type) {
			Geometry::globalBuffers.AddObject(this);
		}
		
		~ObjectInstance() {
			ClearGeometries();
			RemoveLightSources();
			Geometry::globalBuffers.RemoveObject(this);
		}
		
		#pragma endregion
		
		#pragma region Write to device
		
		void GenerateGeometries() {
			if (generateFunc && !generated) generateFunc(this);
			generated = true;
			WriteGeometriesInformation();
		}
		
		void WriteGeometriesInformation() {
			for (auto* geom : geometries) {
				if (!geom->geometryInfoInitialized) geom->SetGeometryInfo(objectOffset, geom->material);
			}
		}
		
		void PushGeometries(Device* device, VkCommandBuffer cmdBuffer) {
			if (geometriesDirty) {
				GenerateGeometries();
				for (auto* geom : geometries) {
					geom->Push(device, cmdBuffer);
				}
			}
			geometriesDirty = false;
		}
		
		void WriteMatrices(const glm::dmat4& viewMatrix) {
			modelViewMatrix = glm::mat4(viewMatrix * transform);
			normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));
			rayTracingModelViewMatrix = glm::transpose(modelViewMatrix);
			
			Geometry::globalBuffers.WriteObject(this);
		
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
		
		Geometry* AddGeometry(uint32_t vertexCount, uint32_t indexCount, uint32_t material = 0) {
			if (!isProcedural.has_value()) isProcedural = false;
			if (isProcedural.value()) {
				LOG_ERROR("An sphere/procedural object cannot contain other triangle geometries")
				return nullptr;
			}
			return geometries.emplace_back(new Geometry(vertexCount, indexCount, material));
		}
		
		Geometry* AddProceduralGeometry(uint32_t count, uint32_t material = 0) {
			if (!isProcedural.has_value()) isProcedural = true;
			if (!isProcedural.value()) {
				LOG_ERROR("A procedural object cannot contain triangle geometries")
				return nullptr;
			}
			return geometries.emplace_back(new Geometry(count, 0, material, true));
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
		
		void SetSphereGeometry(float radius, glm::vec4 color = {0,0,0,0}, uint32_t material = 0, float custom1 = 0) {
			if (!isProcedural.has_value()) isProcedural = true;
			if (!isProcedural.value()) {
				LOG_ERROR("A sphere cannot contain triangle geometries")
				return;
			}
			if (geometries.size() == 0) {
				geometries.emplace_back(new Geometry(1, 0, material, true));
			}
			geometries[0]->SetProceduralVertex(0, glm::vec3(-radius), glm::vec3(+radius), color, custom1);
		}
		
		void SetSphereLightSource(
			float radius,
			glm::f32 lightIntensity,
			glm::vec3 lightColor = {1,1,1},
			glm::u32 lightType = 1,
			glm::u32 lightAttributes = 0,
			float custom1 = 0,
			glm::vec4 geomColor = {0,0,0,0}
		) {
			rayTracingMask = RAY_TRACING_TYPE_EMITTER;
			lightSources.clear();
			auto* lightSource = AddLightSource({0,0,0}, lightIntensity, lightColor, lightType, lightAttributes, radius);
			if (radius > 0) SetSphereGeometry(radius, geomColor, lightSource->lightOffset, custom1);
		}
		
		#pragma endregion
		
		#pragma region Clear
		
		void RemoveGeometries() {
			for (auto* geom : geometries) {
				delete geom;
			}
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
			rayTracingBlasIndex = -1;
			rayTracingInstanceIndex = -1;
			RemoveGeometries();
		}
		
		#pragma endregion
		
		#pragma region Getters/Setters
		
		glm::dmat4 GetWorldTransform() const {
			return transform;
		}
		
		int GetFirstGeometryOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0]->geometryOffset;
		}
		
		std::string& GetObjectType() {return objectType;}
		
		bool IsProcedural() const {return isProcedural.has_value() && isProcedural.value();}
		
		uint32_t GetObjectOffset() const {return objectOffset;}
		
		void SetRayTracingMask(uint32_t i) {rayTracingMask = i;}
		uint32_t GetRayTracingMask() const {return rayTracingMask;}
		
		void SetRayTracingBlasIndex(int i) {rayTracingBlasIndex = i;}
		int GetRayTracingBlasIndex() const {return rayTracingBlasIndex;}
		
		void SetRayTracingInstanceIndex(int i) {rayTracingInstanceIndex = i;}
		int GetRayTracingInstanceIndex() const {return rayTracingInstanceIndex;}
		
		void SetGeometriesDirty() {geometriesDirty = true;}
		bool IsGeometriesDirty() const {return geometriesDirty;}
		
		glm::mat3x4& GetRayTracingModelViewMatrix() {
			return rayTracingModelViewMatrix;
		}
		
		int CountGeometries() const {return geometries.size();}
		
		std::vector<Geometry*>& GetGeometries() {return geometries;}
		std::vector<LightSource*>& GetLightSources() {return lightSources;}
		
		#pragma endregion
		
		#pragma region Realtime / Per-Frame Updates
		
		void SetWorldTransform(const glm::dmat4 worldSpaceTransform) {
			transform = worldSpaceTransform;
		}
		
		void SetObjectCustomData(const glm::vec3& custom3, const glm::vec4& custom4 = {0,0,0,0}) {
			this->custom3 = custom3;
			this->custom4 = custom4;
		}
		
		void SetVertex(uint32_t geomIndex, uint32_t vertIndex, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color) {
			geometries[geomIndex]->SetVertex(vertIndex, pos, normal, uv, color);
			SetGeometriesDirty();
		}
		
		void SetProceduralVertex(uint32_t geomIndex, uint32_t vertIndex, glm::vec3 aabbMin, glm::vec3 aabbMax, const glm::vec4& color, float custom1 = 0) {
			geometries[geomIndex]->SetProceduralVertex(vertIndex, aabbMin, aabbMax, color, custom1);
			SetGeometriesDirty();
		}
		
		#pragma endregion
		
		#pragma region Active/Inactive
		
		bool HasActiveGeometries() const {
			for (auto* g : geometries) {
				if (g->active) return true;
			}
			return false;
		}
		
		bool IsActive() const {
			return !markedForDeletion && active;
		}
		
		void Enable() {
			if (!active) geometriesDirty = true;
			active = true;
		}
		
		void Disable() {
			if (active) geometriesDirty = true;
			active = false;
		}
		
		bool IsGenerated() const {
			return generated;
		}
		
		void SetGenerated() {
			if (!generated) geometriesDirty = true;
			generated = true;
		}
		
		bool IsMarkedForDeletion() const {
			return markedForDeletion;
		}
		
		void MarkForDeletion() {
			markedForDeletion = true;
		}
		
		#pragma endregion
		
	};
	
}
