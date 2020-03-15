#pragma once

#include <v4d.h>
#include "Geometry.h"

namespace v4d::graphics {

	struct V4DLIB ObjectInstance {
		glm::dmat4 transform {1};
		std::vector<Geometry*> geometries {};
		std::vector<LightSource*> lightSources {};
		void (*generateGeometriesFunc)(ObjectInstance*) = nullptr;
		
		// Cached data
		uint32_t objectOffset = 0;
		uint32_t rayTracingMask = 0x01;
		int rayTracingBlasIndex = -1;
		int rayTracingInstanceIndex = -1;
		glm::mat4 modelViewMatrix {1};
		glm::mat3x4 rayTracingModelViewMatrix {1};
		// glm::mat3 normalMatrix {1}; // This does not work... need to do more debugging...
		bool geometriesDirty = true;
		
		ObjectInstance() {
			Geometry::globalBuffers.AddObject(this);
		}
		
		~ObjectInstance() {
			ClearGeometries();
			RemoveLightSources();
			Geometry::globalBuffers.RemoveObject(this);
		}
		
		void GenerateGeometries() {
			if (generateGeometriesFunc) generateGeometriesFunc(this);
			WriteGeometriesInformation();
		}
		
		void WriteGeometriesInformation() {
			for (auto* geom : geometries) {
				if (!geom->geometryInfoInitialized) geom->SetGeometryInfo(objectOffset, geom->materialIndex);
			}
		}
		
		Geometry* AddGeometry(uint32_t vertexCount, uint32_t indexCount, uint32_t materialIndex = 0) {
			return geometries.emplace_back(new Geometry(vertexCount, indexCount, materialIndex));
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
		
		void RemoveGeometries() {
			for (auto* geom : geometries) {
				delete geom;
			}
			geometries.clear();
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
		
		void PushGeometries(Device* device, VkCommandBuffer cmdBuffer) {
			if (geometriesDirty) {
				if (geometries.size() == 0) GenerateGeometries();
				for (auto* geom : geometries) {
					geom->Push(device, cmdBuffer);
				}
			}
			geometriesDirty = false;
		}
		
		void WriteMatrices(const glm::dmat4& viewMatrix) {
			modelViewMatrix = glm::mat4(viewMatrix * transform);
			rayTracingModelViewMatrix = glm::transpose(modelViewMatrix);
			// normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix))); // This does not work... need to do more debugging...
			
			Geometry::globalBuffers.WriteObject(this);
		
			for (auto* lightSource : lightSources) {
				lightSource->viewSpacePosition = viewMatrix * transform * glm::dvec4(lightSource->position, 1);
				Geometry::globalBuffers.WriteLight(lightSource);
			}
		}
		
		int GetFirstGeometryOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0]->geometryOffset;
		}
		
	};
	
}
