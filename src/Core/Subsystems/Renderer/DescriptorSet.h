#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class DescriptorSet
{
 public:
    DescriptorSet();
    ~DescriptorSet();

 private:
    std::vector<VkDescriptorSet> _vkDescriptorSets = {};
};
