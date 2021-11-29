#include "./DescriptorPool.h"
#include "./../../utils.h"
#include "LogicalDevice.h"

#include <array>

extern std::unique_ptr<LogicalDevice> g_logicalDevice;

DescriptorPool::DescriptorPool(const uint32_t maxFramesInFlight)
{
    const std::array<VkDescriptorPoolSize, 2> poolSizes =
    {
        VkDescriptorPoolSize
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = maxFramesInFlight,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 32 * 1024,
        },
    };

    const VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .maxSets = static_cast<uint32_t>(poolSizes.size() * maxFramesInFlight),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    CHECK("Descriptor pool", vkCreateDescriptorPool(g_logicalDevice->vkDevice(), &poolInfo, VK_NULL_HANDLE, &_descriptorPool));
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(g_logicalDevice->vkDevice(), _descriptorPool, VK_NULL_HANDLE);
}

const VkDescriptorPool DescriptorPool::vkDescriptorPool() const
{
    return _descriptorPool;
}
