#pragma once

#include <vulkan/vulkan.h>

#include "../../utils.h"
#include "PhysicalDevice.h"

class LogicalDevice
{
 public:
    LogicalDevice();
    const VkDevice vkDevice() const;
    const VkQueue vkGraphicsQueue() const;
    const VkQueue vkPresentQueue() const;

 private:
    VkDevice    _vkDevice           = VK_NULL_HANDLE;
    VkQueue     _vkGraphicsQueue    = VK_NULL_HANDLE;
    VkQueue     _vkPresentQueue     = VK_NULL_HANDLE;
};
