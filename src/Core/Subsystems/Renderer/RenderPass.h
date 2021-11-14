#pragma once

#include <vulkan/vulkan.h>

#include "../../utils.h"

class RenderPass
{
 public:
    RenderPass(const VkDevice& vkDevice, const VkFormat& colorFormat);
    VkRenderPass& operator()();
    ~RenderPass();

 private:
    const VkDevice& _vkDevice;
    VkRenderPass _vkRenderPass = VK_NULL_HANDLE;
};
