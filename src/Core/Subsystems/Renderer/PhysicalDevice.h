#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <optional>

#include "../../utils.h"
#include "Swapchain.h"

class PhysicalDevice
{
 public:
    PhysicalDevice();
    const VkPhysicalDevice vkPhysicalDevice() const;
    const QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;
    const std::array<const char*, 1>& deviceExtensions() const;

 private:
    bool isPhysicalDeviceSuitable(const VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device) const;

    VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;
    const std::array<const char*, 1> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
};
