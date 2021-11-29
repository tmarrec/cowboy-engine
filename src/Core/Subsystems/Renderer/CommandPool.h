#pragma once

#include <vulkan/vulkan.h>

class CommandPool
{
 public:
    CommandPool();
    ~CommandPool();

 private:
    VkCommandPool _vkCommandPool = VK_NULL_HANDLE;
};
