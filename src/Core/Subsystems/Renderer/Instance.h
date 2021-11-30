#pragma once

#include <vulkan/vulkan.h>

class Instance
{
 public:
    Instance();
    ~Instance();
    const VkInstance    vkInstance()    const;
    const VkSurfaceKHR  vkSurface()     const;
    VkSurfaceKHR*       vkSurfacePtr();

 private:
    VkInstance      _vkInstance     = VK_NULL_HANDLE;
    VkSurfaceKHR    _vkSurface      = VK_NULL_HANDLE;
};
