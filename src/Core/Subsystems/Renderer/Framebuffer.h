#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Framebuffer
{
 public:
    Framebuffer(const VkRenderPass vkRenderPass, const std::vector<VkImageView>& attachments, const uint32_t width, const uint32_t height, const uint32_t arrayLayers);
    ~Framebuffer();

 private:
    VkFramebuffer _vkFramebuffer = VK_NULL_HANDLE;
};
