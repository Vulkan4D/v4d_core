#pragma once

#include <v4d.h>
#include "Geometry.h"

namespace v4d::graphics {

	struct V4DLIB ObjectInstance {
		glm::dmat4 transform {1};
		std::vector<Geometry*> geometries {};
		void (*generateGeometriesFunc)(ObjectInstance*) = nullptr;
		
		uint32_t rayTracingMask = 0x01;
		int rayTracingInstanceIndex = -1;
		
		~ObjectInstance() {
			RemoveGeometries();
		}
		
		void GenerateGeometries() {
			if (generateGeometriesFunc) generateGeometriesFunc(this);
		}
		
		Geometry* AddGeometry(uint32_t vertexCount, uint32_t indexCount) {
			return geometries.emplace_back(new Geometry(vertexCount, indexCount));
		}
		
		void RemoveGeometries() {
			for (auto* geom : geometries) {
				delete geom;
			}
			geometries.clear();
		}
		
		void PushGeometries(Device* device, VkCommandBuffer cmdBuffer) {
			if (geometries.size() == 0) GenerateGeometries();
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
