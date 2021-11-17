#pragma once

#include <vulkan/vulkan.h>

class DescriptorPool
{
 public:
    DescriptorPool(const uint32_t maxFramesInFlight);
    ~DescriptorPool();

 private:
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
};
