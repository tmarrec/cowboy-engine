#include "./Swapchain.h"

#include <vulkan/vulkan_core.h>
#include "PhysicalDevice.h"
#include "LogicalDevice.h"

extern Window g_Window;
inline std::shared_ptr<PhysicalDevice> g_physicalDevice;
inline std::shared_ptr<LogicalDevice> g_logicalDevice;

// Create the Vulkan swapchain
Swapchain::Swapchain(const VkSurfaceKHR surface)
{
    // Get the informations needed for the swapchain
    const SwapchainSupportDetails swapchainSupport = querySupport(g_physicalDevice->vkPhysicalDevice(), surface);
    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
    const VkPresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
    _imageCount = [&]()
    {
        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }
        return imageCount;
    }();
    _vkSurfaceFormat = surfaceFormat.format;
    _vkExtent = chooseExtent(swapchainSupport.capabilities);

    // Fill the swapchain informations
    const VkSwapchainCreateInfoKHR createInfo = [&]()
    {
        VkSwapchainCreateInfoKHR createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .surface = surface,
            .minImageCount = _imageCount,
            .imageFormat = _vkSurfaceFormat,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = _vkExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapchainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = _vkSwapchain,
        };
        auto indices = g_physicalDevice->findQueueFamilies(g_physicalDevice->vkPhysicalDevice());
        if (indices.graphics != indices.present)
        {
            const std::array<uint32_t, 2> queueFamilyIndices = {indices.graphics.value(), indices.present.value()};
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
        }
        return createInfo;
    }();


    // Create the swapchain
    if (vkCreateSwapchainKHR(g_logicalDevice->vkDevice(), &createInfo, VK_NULL_HANDLE, &_vkSwapchain) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create swapchain.");
    }
    INFO("Swapchain successfully created.");
}

// Get the swapchain surface format, aiming for RGBA 32bits sRGB 
VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    WARNING("Unable to find desired swapchain surface formats. Will use the first found.");
    return availableFormats[0];
}

// Get the swapchain present mode, aiming for MAILBOX mode
VkPresentModeKHR Swapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    WARNING("Unable to find VK_PRESENT_MODE_MAILBOX_KHR swapchain present mode. Will use VK_PRESENT_MODE_IMMEDIATE_KHR.");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

// Choose the swap extent based on the window manager informations
VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        return [&]()
        {
            uint32_t width, height;
            g_Window.windowGetFramebufferSize(width, height);
            width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return VkExtent2D{width, height};
        }();
    }
}

// Get the details of the device swapchain support
const SwapchainSupportDetails Swapchain::querySupport(const VkPhysicalDevice device, const VkSurfaceKHR surface) const
{
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, VK_NULL_HANDLE);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

const uint32_t Swapchain::imageCount() const
{
    return _imageCount;
}

const VkFormat Swapchain::format() const
{
    return _vkSurfaceFormat;
}

const VkExtent2D Swapchain::extent() const
{
    return _vkExtent;
}

// Getter to the vulkan swapchain object
VkSwapchainKHR& Swapchain::vkSwapchain()
{
    return _vkSwapchain;
}
