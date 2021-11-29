#include "CommandBuffer.h"
#include <memory>

#include "LogicalDevice.h"

extern std::unique_ptr<LogicalDevice>   g_logicalDevice;

CommandBuffer::CommandBuffer(const VkCommandPool vkCommandPool)
{
    const VkCommandBufferAllocateInfo commandBufferAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .commandPool = vkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    CHECK("Command buffer", vkAllocateCommandBuffers(g_logicalDevice->vkDevice(), &commandBufferAllocateInfo, &_vkCommandBuffer));
}

void CommandBuffer::begin() const
{
    const VkCommandBufferBeginInfo commandBufferBeginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = VK_NULL_HANDLE,
    };
    vkBeginCommandBuffer(_vkCommandBuffer, &commandBufferBeginInfo);
}

void CommandBuffer::end() const
{
    vkEndCommandBuffer(_vkCommandBuffer);
}
