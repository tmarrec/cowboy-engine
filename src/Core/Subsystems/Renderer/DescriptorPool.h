#pragma once

#include <vulkan/vulkan.h>

class DescriptorPool
{
 public:
    DescriptorPool(const uint32_t maxFramesInFlight);
    ~DescriptorPool();
    const VkDescriptorPool vkDescriptorPool() const;

 private:
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
};
