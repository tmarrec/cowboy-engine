#pragma once

#include <vulkan/vulkan.h>

#include "../../utils.h"
#include "PhysicalDevice.h"

class LogicalDevice
{
 public:
    LogicalDevice();
    const VkDevice vkDevice() const;

 private:
    VkDevice _vkDevice = VK_NULL_HANDLE;
};
