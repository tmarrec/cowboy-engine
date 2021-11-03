#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <array>

#include "../../utils.h"
#include "../Window/WindowManager.h"

struct QueueFamilyIndices
{
	std::optional<std::uint32_t> graphics;
	std::optional<std::uint32_t> present;

	bool isComplete()
	{
		return graphics.has_value() && present.has_value();
	}
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain
{
 public:
    void create(const VkPhysicalDevice physicalDevice, const VkDevice device, const VkSurfaceKHR surface, const QueueFamilyIndices& indices);
    SwapchainSupportDetails querySupport(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface);
    const std::uint32_t imageCount() const;
    const VkFormat format() const;
    const VkExtent2D extent() const;
    VkSwapchainKHR& operator()();

 private:
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR capabilities);

    VkSwapchainKHR  _swapchain      = VK_NULL_HANDLE;
    VkFormat        _surfaceFormat  = {};
    VkExtent2D      _extent         = {};
    std::uint32_t   _imageCount     = 0;
};
