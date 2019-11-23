#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {

	class V4DLIB Instance : public xvk::Interface::InstanceInterface {
	private:
		std::vector<PhysicalDevice*> availablePhysicalDevices;

		void LoadAvailablePhysicalDevices();

	protected:
		vulkan::Loader* vulkanLoader;

	public:
		Instance(vulkan::Loader* loader, const char* applicationName, uint applicationVersion, bool logging = false);
		virtual ~Instance();

		VkInstance GetHandle() const;

		PhysicalDevice* SelectSuitablePhysicalDevice(const std::function<void(int&, PhysicalDevice*)>& suitabilityFunc);

	};
}
