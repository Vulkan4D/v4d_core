/*
 * Window abstraction of GLFW for use with the Vulkan API
 * Part of the Vulkan4D open-source game engine under the LGPL license
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 * 
 * This class is an abstraction of GLFW methods for quick Vulkan integration
 */
#pragma once

#include <v4d.h>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include "utilities/graphics/vulkan/Loader.h"

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
		std::map<std::string, std::function<void(double,double)>> scrollCallbacks {};
		std::map<std::string, std::function<void(unsigned int)>> charCallbacks {};
		std::map<std::string, std::function<void()>> windowCloseCallbacks {};

		static void ResizeCallback(GLFWwindow* handle, int newWidth, int newHeight);
		static void KeyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* handle, int button, int action, int mods);
		static void ScrollCallback(GLFWwindow* handle, double x, double y);
		static void CharCallback(GLFWwindow* handle, unsigned int c);
		static void WindowCloseCallback(GLFWwindow* handle);

		static int GetNextIndex();

		static void ActivateWindowSystem();
		static void DeactivateWindowSystem();

	public:
		Window(const std::string& title, int width, int height, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
		~Window();

		[[nodiscard]] VkSurfaceKHR CreateVulkanSurface(VkInstance instance);
		
		void FillRequiredVulkanInstanceExtensions(std::vector<const char*>& requiredInstanceExtensions) const;
		
		void AddResizeCallback(std::string name, std::function<void(int,int)>&& callback);
		void RemoveResizeCallback(std::string name);

		void AddKeyCallback(std::string name, std::function<void(int,int,int,int)>&& callback);
		void RemoveKeyCallback(std::string name);

		void AddMouseButtonCallback(std::string name, std::function<void(int,int,int)>&& callback);
		void RemoveMouseButtonCallback(std::string name);
		
		void AddScrollCallback(std::string name, std::function<void(double,double)>&& callback);
		void RemoveScrollCallback(std::string name);
		
		void AddCharCallback(std::string name, std::function<void(unsigned int)>&& callback);
		void RemoveCharCallback(std::string name);
		
		void AddWindowCloseCallback(std::string name, std::function<void()>&& callback);
		void RemoveWindowCloseCallback(std::string name);
		
		bool IsActive();

		GLFWwindow* GetHandle() const;
		
		int GetWidth() const;
		int GetHeight() const;
		
		void SetTitle(const char* title);
		void SetTitle(const std::string& title);

		void WaitEvents();
		
		inline void Close() {
			glfwSetWindowShouldClose(handle, 1);
		}
		
	};
}
