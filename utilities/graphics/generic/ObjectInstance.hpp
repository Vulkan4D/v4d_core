#pragma once

#include <v4d.h>
#include "Geometry.h"

namespace v4d::graphics {

	struct V4DLIB ObjectInstance {
		glm::dmat4 transform {1};
		std::vector<Geometry*> geometries {};
		std::vector<LightSource*> lightSources {};
		void (*generateFunc)(ObjectInstance*) = nullptr;
		
		uint32_t rayTracingMask = 0x01;
		int rayTracingInstanceIndex = -1;
		
		~ObjectInstance() {
			Clear();
		}
		
		void Generate() {
			if (generateFunc) generateFunc(this);
		}
		
		Geometry* AddGeometry(uint32_t vertexCount, uint32_t indexCount) {
			return geometries.emplace_back(new Geometry(vertexCount, indexCount));
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
		
		void Clear() {
			RemoveGeometries();
			RemoveLightSources();
		}
		
		void PushGeometries(Device* device, VkCommandBuffer cmdBuffer) {
			if (geometries.size() == 0) Generate();
			for (auto* geom : geometries) {
				geom->Push(device, cmdBuffer);
			}
		}
		
		glm::mat3x4 GetViewTransform(const glm::dmat4& viewMatrix) {
			return glm::transpose(glm::mat4(viewMatrix * transform));
		}
		
		int GetFirstGeometryOffset() const {
			if (!geometries.size()) return 0;
			return geometries[0]->geometryOffset;
		}
		
	};
	
}
