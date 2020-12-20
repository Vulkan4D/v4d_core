#pragma once

#include <v4d.h>

namespace v4d::scene {
	template<class ModelDataType>
	class ModelLoader {
		virtual bool Load() = 0;
		virtual void Generate(v4d::graphics::RenderableGeometryEntity*, v4d::graphics::vulkan::Device*) = 0;
	public:
		std::shared_ptr<ModelDataType> modelData;
		bool loaded = false;
		void operator()(v4d::graphics::RenderableGeometryEntity* entity = nullptr, v4d::graphics::vulkan::Device* device = nullptr) {
			if (!loaded) {
				if (Load()) loaded = true;
			}
			if (loaded && entity && device) {
				Generate(entity, device);
			}
		}
		ModelLoader(const ModelLoader& original) : modelData(original.modelData) {}
		ModelLoader(ModelLoader&& original) : modelData(original.modelData) {}
		ModelLoader() : modelData(std::make_shared<ModelDataType>()) {}
		virtual ~ModelLoader(){}
	};
}
