#include <v4d.h>

using namespace v4d::graphics;

std::unordered_map<int, Window*> Window::windows {};

int Window::GetNextIndex() {
	static std::atomic<int> nextIndex = 0;
	return nextIndex++;
}

void Window::ActivateWindowSystem() {
	// Init GLFW
	if (!glfwInit())
		throw std::runtime_error("GLFW Init Failed");
	
	// Hints
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Check for Vulkan support
	if (!glfwVulkanSupported())
		throw std::runtime_error("Vulkan is not supported");
}

void Window::DeactivateWindowSystem() {
	glfwTerminate();
}

void Window::ResizeCallback(GLFWwindow* handle, int newWidth, int newHeight) {
	auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
	window->RefreshSize();
	for (auto[name, callback] : window->resizeCallbacks) {
		callback(newWidth, newHeight);
	}
}

void Window::KeyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods) {
	auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
	for (auto[name, callback] : window->keyCallbacks) {
		callback(key, scancode, action, mods);
	}
}

void Window::MouseButtonCallback(GLFWwindow* handle, int button, int action, int mods) {
	auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
	for (auto[name, callback] : window->mouseButtonCallbacks) {
		callback(button, action, mods);
	}
}

Window::Window(const std::string& title, int width, int height, GLFWmonitor* monitor, GLFWwindow* share) : index(GetNextIndex()), width(width), height(height) {
	if (windows.size() == 0) ActivateWindowSystem();
	windows.emplace(index, this);
	handle = glfwCreateWindow(width, height, title.c_str(), monitor, share);
	glfwSetWindowUserPointer(handle, this);
	glfwSetFramebufferSizeCallback(handle, ResizeCallback);
	glfwSetKeyCallback(handle, KeyCallback);
	glfwSetMouseButtonCallback(handle, MouseButtonCallback);
}

Window::~Window() {
	glfwDestroyWindow(handle);
	windows.erase(index);
	if (windows.size() == 0) DeactivateWindowSystem();
}

VkSurfaceKHR Window::CreateVulkanSurface(VkInstance instance) {
	VkSurfaceKHR surface;
	if (VkResult err = glfwCreateWindowSurface(instance, handle, nullptr, &surface); err != VK_SUCCESS) {
		LOG_ERROR(err)
		throw std::runtime_error("Failed to create Vulkan Surface");
	}
	return surface;
}

void Window::GetRequiredVulkanInstanceExtensions(std::vector<const char*>& requiredInstanceExtensions) const {
	uint glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	requiredInstanceExtensions.reserve(glfwExtensionCount);
	for (uint i = 0; i < glfwExtensionCount; i++) {
		requiredInstanceExtensions.push_back(glfwExtensions[i]);
	}
}

void Window::AddResizeCallback(std::string name, std::function<void(int,int)>&& callback) {
	resizeCallbacks[name] = std::forward<std::function<void(int,int)>>(callback);
}

void Window::RemoveResizeCallback(std::string name) {
	resizeCallbacks.erase(name);
}

void Window::AddKeyCallback(std::string name, std::function<void(int,int,int,int)>&& callback) {
	keyCallbacks[name] = std::forward<std::function<void(int,int,int,int)>>(callback);
}

void Window::RemoveKeyCallback(std::string name) {
	keyCallbacks.erase(name);
}

void Window::AddMouseButtonCallback(std::string name, std::function<void(int,int,int)>&& callback) {
	mouseButtonCallbacks[name] = std::forward<std::function<void(int,int,int)>>(callback);
}

void Window::RemoveMouseButtonCallback(std::string name) {
	mouseButtonCallbacks.erase(name);
}

bool Window::IsActive() {
	return !glfwWindowShouldClose(handle);
}

GLFWwindow* Window::GetHandle() const {
	return handle;
}

int Window::GetWidth() const {
	return width;
}

int Window::GetHeight() const {
	return height;
}

void Window::RefreshSize() {
	glfwGetFramebufferSize(handle, &width, &height);
}

void Window::WaitEvents() {
	RefreshSize();
	glfwWaitEvents();
}