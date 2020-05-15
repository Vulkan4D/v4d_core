#pragma once
#include <v4d.h>

class V4DLIB V4D_Input {
	V4D_MODULE_CLASS_H(V4D_Input
		,OrderIndex
		,Init
		,CallbackName
		,CharCallback
		,KeyCallback
		,ScrollCallback
		,MouseButtonCallback
		,Update
	)
	
	V4D_MODULE_FUNC(int, OrderIndex)
	V4D_MODULE_FUNC(void, Init, v4d::graphics::Window*, v4d::graphics::Renderer*)
	V4D_MODULE_FUNC(std::string, CallbackName)
	V4D_MODULE_FUNC(void, CharCallback, unsigned int c)
	V4D_MODULE_FUNC(void, KeyCallback, int key, int scancode, int action, int mods)
	V4D_MODULE_FUNC(void, ScrollCallback, double x, double y)
	V4D_MODULE_FUNC(void, MouseButtonCallback, int button, int action, int mods)
	V4D_MODULE_FUNC(void, Update, double deltaTime)

	void AddCallbacks(v4d::graphics::Window* window) {
		window->AddKeyCallback(this->CallbackName(), [this](int key, int scancode, int action, int mods){
			this->KeyCallback(key, scancode, action, mods);
		});
		window->AddMouseButtonCallback(this->CallbackName(), [this](int button, int action, int mods){
			this->MouseButtonCallback(button, action, mods);
		});
		window->AddScrollCallback(this->CallbackName(), [this](double x, double y){
			this->ScrollCallback(x, y);
		});
		window->AddCharCallback(this->CallbackName(), [this](unsigned int c){
			this->CharCallback(c);
		});
	}
	
	void RemoveCallbacks(v4d::graphics::Window* window) {
		window->RemoveKeyCallback(this->CallbackName());
		window->RemoveMouseButtonCallback(this->CallbackName());
	}
	
};
