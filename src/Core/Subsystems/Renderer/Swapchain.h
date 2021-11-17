#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <array>

#include "./../../utils.h"
#include "./../Window/Window.h"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	bool isComplete() const
	{
		return graphics.has_value() && present.has_value();
	}
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class Swapchain
{
 public:
    Swapchain(const VkSurfaceKHR surface);
    const SwapchainSupportDetails querySupport(const VkPhysicalDevice device, const VkSurfaceKHR surface) const;
    const uint32_t imageCount() const;
    const VkFormat format() const;
    const VkExtent2D extent() const;
    VkSwapchainKHR& vkSwapchain();

 private:
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR capabilities);

    VkSwapchainKHR  _vkSwapchain        = VK_NULL_HANDLE;
    VkFormat        _vkSurfaceFormat    = {};
    VkExtent2D      _vkExtent           = {};
    uint32_t        _imageCount         = 0;
};
