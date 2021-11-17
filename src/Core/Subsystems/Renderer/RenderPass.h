#pragma once

#include <vulkan/vulkan.h>

#include "../../utils.h"

class RenderPass
{
 public:
    RenderPass();
    const VkRenderPass& vkRenderPass() const;
    ~RenderPass();

 private:
    VkRenderPass _vkRenderPass = VK_NULL_HANDLE;
};
