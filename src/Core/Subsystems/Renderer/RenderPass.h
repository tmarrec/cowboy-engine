#pragma once

#include <vulkan/vulkan.h>

#include "../../utils.h"

class RenderPass
{
 public:
    RenderPass();
    const VkRenderPass& vkRenderPass() const;
    ~RenderPass();
    void begin(const VkCommandBuffer vkCommandBuffer, const VkFramebuffer vkFramebuffer) const;
    void end(const VkCommandBuffer vkCommandBuffer) const;

 private:
    VkRenderPass _vkRenderPass = VK_NULL_HANDLE;
};
