#pragma once

#include <v4d.h>

namespace v4d::scene {
	template<class ModelDataType>
	class ModelLoader {
	public:
		virtual void Generate(v4d::graphics::RenderableGeometryEntity*) = 0;
		std::shared_ptr<ModelDataType> modelData;
		virtual void Load() = 0;
		void operator()(v4d::graphics::RenderableGeometryEntity* entity) {
			Generate(entity);
		}
		ModelLoader(const ModelLoader& original) : modelData(original.modelData) {}
		ModelLoader(ModelLoader&& original) : modelData(original.modelData) {}
		ModelLoader() : modelData(std::make_shared<ModelDataType>()) {}
		virtual ~ModelLoader(){}
	};
}
