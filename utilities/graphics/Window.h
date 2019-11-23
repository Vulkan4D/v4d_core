#pragma once
#include <v4d.h>

// GLFW
#include <GLFW/glfw3.h>

namespace v4d::graphics {
	
	class V4DLIB Window {
	private:
		int index;
		int width, height;

		GLFWwindow *handle;
		
		static std::unordered_map<int, Window*> windows;
		
		std::map<std::string, std::function<void(int,int)>> resizeCallbacks {};
		std::map<std::string, std::function<void(int,int,int,int)>> keyCallbacks {};
		std::map<std::string, std::function<void(int,int,int)>> mouseButtonCallbacks {};

		static void ResizeCallback(GLFWwindow* handle, int newWidth, int newHeight);
		static void KeyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* handle, int button, int action, int mods);

		static int GetNextIndex();

		static void ActivateWindowSystem();
		static void DeactivateWindowSystem();

	public:
		Window(const std::string& title, int width, int height, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
		~Window();

		VkSurfaceKHR CreateVulkanSurface(VkInstance instance);
		
		void GetRequiredVulkanInstanceExtensions(std::vector<const char*>& requiredInstanceExtensions) const;
		
		void AddResizeCallback(std::string name, std::function<void(int,int)>&& callback);
		void RemoveResizeCallback(std::string name);

		void AddKeyCallback(std::string name, std::function<void(int,int,int,int)>&& callback);
		void RemoveKeyCallback(std::string name);

		void AddMouseButtonCallback(std::string name, std::function<void(int,int,int)>&& callback);
		void RemoveMouseButtonCallback(std::string name);
		
		bool IsActive();

		GLFWwindow* GetHandle() const;
		
		int GetWidth() const;
		int GetHeight() const;

		void RefreshSize();

		void WaitEvents();

	};
}
