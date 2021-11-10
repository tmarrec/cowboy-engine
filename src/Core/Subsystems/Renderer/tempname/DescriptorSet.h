#pragma once

#include <vulkan/vulkan.h>

class DescriptorSet
{
 public:
    DescriptorSet(VkDevice& device, const uint32_t maxFramesInFlight);
    ~DescriptorSet();

 private:
    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
    VkDevice& _device;
};
