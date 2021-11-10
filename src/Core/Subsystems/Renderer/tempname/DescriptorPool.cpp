#include "./DescriptorPool.h"
#include "./../../../utils.h"

#include <array>

DescriptorPool::DescriptorPool(VkDevice& device, const uint32_t maxFramesInFlight)
: _device {device}
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
            .descriptorCount = maxFramesInFlight,
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

    if (vkCreateDescriptorPool(_device, &poolInfo, VK_NULL_HANDLE, &_descriptorPool) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create descriptor pool.");
    }
    INFO("Descriptor pool successfully created.");
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, VK_NULL_HANDLE);
}
