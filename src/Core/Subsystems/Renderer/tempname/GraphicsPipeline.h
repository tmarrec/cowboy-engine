#pragma once

#include <vulkan/vulkan.h>

class GraphicsPipeline
{
 public:
    GraphicsPipeline(const VkDevice& device);
    ~GraphicsPipeline();

 private:
    VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
    const VkDevice& _device;
};
