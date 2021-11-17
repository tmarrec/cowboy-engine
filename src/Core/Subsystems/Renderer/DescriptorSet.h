#pragma once

#include <vulkan/vulkan.h>

class DescriptorSet
{
 public:
    DescriptorSet(const uint32_t maxFramesInFlight);
    ~DescriptorSet();

 private:
    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
};
