#pragma once

#include <v4d.h>
#include <memory>
#include "utilities/graphics/vulkan/Device.h"
#include "utilities/graphics/RenderableGeometryEntity.h"

namespace v4d::scene {
	template<class ModelDataType>
	class ModelLoader {
		virtual bool Load() = 0;
		virtual void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) = 0;
	public:
		std::shared_ptr<ModelDataType> modelData;
		std::shared_ptr<bool> loaded;
		void operator()(v4d::graphics::RenderableGeometryEntity* entity = nullptr, v4d::graphics::vulkan::Device* device = nullptr) {
			if (!*loaded) {
				if (Load()) *loaded = true;
			}
			if (*loaded && entity && device) {
				Generate(entity, device);
			}
		}
		ModelLoader(const ModelLoader& original) : modelData(original.modelData), loaded(original.loaded) {}
		ModelLoader(ModelLoader&& original) : modelData(original.modelData), loaded(original.loaded) {}
		ModelLoader() : modelData(std::make_shared<ModelDataType>()), loaded(std::make_shared<bool>(false)) {}
		virtual ~ModelLoader(){}
	};
}
