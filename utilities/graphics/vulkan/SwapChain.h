/*
 * Vulkan SwapChain abstraction
 * Part of the Vulkan4D open-source game engine under the LGPL license - https://github.com/Vulkan4D
 * @author Olivier St-Laurent <olivier@xenon3d.com>
 */
#pragma once
#include <v4d.h>

namespace v4d::graphics::vulkan {
	
	struct V4DLIB SwapChain {
	private:
		Device* device;
		VkSurfaceKHR surface;
		VkSwapchainKHR handle;

	private:
		// Supported configurations
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

	public:
		// SwapChain images
		std::vector<VkImage> images {};
		
		// Selected configuration
		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;
		VkExtent2D extent;

		VkPipelineViewportStateCreateInfo viewportState {};
		VkViewport viewport {};
		VkRect2D scissor {};
		VkSwapchainCreateInfoKHR createInfo {};
		VkImageViewCreateInfo imageViewsCreateInfo {};
		std::vector<VkImageView> imageViews {};

		// Constructor
		SwapChain();
		SwapChain(Device* device, VkSurfaceKHR surface);
		SwapChain(
			Device* device, 
			VkSurfaceKHR surface, 
			uint32_t nbFrames, 
			VkExtent2D preferredExtent, 
			const std::vector<VkSurfaceFormatKHR> preferredFormats, 
			const std::vector<VkPresentModeKHR> preferredPresentModes
		);
		~SwapChain();

		VkSwapchainKHR GetHandle() const;

		void SetConfiguration(uint32_t nbFrames, VkExtent2D preferredExtent, const std::vector<VkSurfaceFormatKHR> preferredFormats, const std::vector<VkPresentModeKHR>& preferredPresentModes);
		void AssignQueues(std::vector<uint32_t> queues);

		void Create(SwapChain* oldSwapChain = nullptr);
		void Destroy();

		void ResolveCapabilities();
		void ResolveFormats();
		void ResolvePresentModes();

		VkExtent2D GetPreferredExtent(VkExtent2D preferredExtent);
		VkSurfaceFormatKHR GetPreferredSurfaceFormat(const std::vector<VkSurfaceFormatKHR> preferredFormats);
		VkPresentModeKHR GetPreferredPresentMode(const std::vector<VkPresentModeKHR>& preferredPresentModes);
			/*
			VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.
			VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games. The moment that the display is refreshed is known as "vertical blank".
			VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.
			VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to implement triple buffering, which allows you to avoid tearing with significantly less latency issues than standard vertical sync that uses double buffering.
			*/
	};
}
