#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices
{
	std::optional<std::uint32_t> graphics;
	std::optional<std::uint32_t> present;

	bool isComplete()
	{
		return graphics.has_value() && present.has_value();
	}
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class RendererManager
{
 public:
    RendererManager();
    ~RendererManager();

 private:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device);

    // Swapchain
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();

    const std::vector<const char*> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance              _vkInstance         = VK_NULL_HANDLE;
    VkPhysicalDevice        _vkPhysicalDevice   = VK_NULL_HANDLE;
    VkSurfaceKHR            _vkSurface          = VK_NULL_HANDLE;
    VkDevice                _vkDevice           = VK_NULL_HANDLE;
    VkQueue                 _vkGraphicsQueue    = VK_NULL_HANDLE;
    VkSwapchainKHR          _vkSwapChain        = VK_NULL_HANDLE;
    VkFormat                _vkSwapChainFormat  = {};
    VkExtent2D              _vkSwapChainExtent  = {};
    std::vector<VkImage>    _vkSwapChainImages  = {};
};
