#pragma once

#include <vulkan/vulkan.h>

class CommandBuffer
{
 public:
    CommandBuffer(const VkCommandPool vkCommandPool);
    void begin() const;
    void end() const;

 private:
    VkCommandBuffer _vkCommandBuffer = VK_NULL_HANDLE;
};
