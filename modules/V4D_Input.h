#pragma once
#include <v4d.h>

class V4DLIB V4D_Input {
	V4D_MODULE_CLASS_HEADER(V4D_Input
		,OrderIndex
		,Init
		,CallbackName
		,CharCallback
		,KeyCallback
		,ScrollCallback
		,MouseButtonCallback
		,Update
	)
	
	V4D_MODULE_FUNC_DECLARE(int, OrderIndex)
	V4D_MODULE_FUNC_DECLARE(void, Init, v4d::graphics::Window*, v4d::graphics::Renderer*, v4d::scene::Scene*)
	V4D_MODULE_FUNC_DECLARE(std::string, CallbackName)
	V4D_MODULE_FUNC_DECLARE(void, CharCallback, unsigned int c)
	V4D_MODULE_FUNC_DECLARE(void, KeyCallback, int key, int scancode, int action, int mods)
	V4D_MODULE_FUNC_DECLARE(void, ScrollCallback, double x, double y)
	V4D_MODULE_FUNC_DECLARE(void, MouseButtonCallback, int button, int action, int mods)
	V4D_MODULE_FUNC_DECLARE(void, Update, double deltaTime)

	void AddCallbacks(v4d::graphics::Window* window) {
		if (this->KeyCallback) {
			window->AddKeyCallback(this->CallbackName(), [this](int key, int scancode, int action, int mods){
				this->KeyCallback(key, scancode, action, mods);
			});
		}
		if (this->MouseButtonCallback) {
			window->AddMouseButtonCallback(this->CallbackName(), [this](int button, int action, int mods){
				this->MouseButtonCallback(button, action, mods);
			});
		}
		if (this->ScrollCallback) {
			window->AddScrollCallback(this->CallbackName(), [this](double x, double y){
				this->ScrollCallback(x, y);
			});
		}
		if (this->CharCallback) {
			window->AddCharCallback(this->CallbackName(), [this](unsigned int c){
				this->CharCallback(c);
			});
		}
	}
	
	void RemoveCallbacks(v4d::graphics::Window* window) {
		window->RemoveKeyCallback(this->CallbackName());
		window->RemoveMouseButtonCallback(this->CallbackName());
		window->RemoveScrollCallback(this->CallbackName());
		window->RemoveCharCallback(this->CallbackName());
	}
	
};
