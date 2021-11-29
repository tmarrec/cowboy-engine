#include "CommandPool.h"
#include <memory>

#include "LogicalDevice.h"

extern std::unique_ptr<LogicalDevice>   g_logicalDevice;
extern std::unique_ptr<PhysicalDevice>  g_physicalDevice;

CommandPool::CommandPool()
{
    const VkCommandPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = g_physicalDevice->findQueueFamilies(g_physicalDevice->vkPhysicalDevice()).graphics.value(),
    };
    CHECK("Command pool", vkCreateCommandPool(g_logicalDevice->vkDevice(), &poolInfo, VK_NULL_HANDLE, &_vkCommandPool));
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(g_logicalDevice->vkDevice(), _vkCommandPool, VK_NULL_HANDLE);
}
