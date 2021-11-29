#include "Framebuffer.h"
#include <memory>

#include "LogicalDevice.h"

extern std::unique_ptr<LogicalDevice>   g_logicalDevice;

Framebuffer::Framebuffer(const VkRenderPass vkRenderPass, const std::vector<VkImageView>& attachments, const uint32_t width, const uint32_t height, const uint32_t arrayLayers)
{
    const VkFramebufferCreateInfo framebufferCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .renderPass = vkRenderPass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = width,
        .height = height,
        .layers = arrayLayers,
    };
    CHECK("Framebuffer", vkCreateFramebuffer(g_logicalDevice->vkDevice(), &framebufferCreateInfo, VK_NULL_HANDLE, &_vkFramebuffer));
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(g_logicalDevice->vkDevice(), _vkFramebuffer, VK_NULL_HANDLE);
}
